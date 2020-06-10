#! /usr/bin/env python

from io import open # python3-ism, for encoding support
import re
import sys

class Struct(object):
    def __init__(self, name):    
        self.name = name
        self.fields = []

    def addField(self, name):
        self.fields.append(name)

def parseStructField(line, struct):
    match = re.search('\s*([A-Za-z0-9_]+)\s*([A-Za-z0-9_]+);', line)
    if match:
        type = match.group(1)
        if type == 'TMetadataString' or type == 'TMetadataExtKey':
            struct.addField(match.group(2))
        
def load(filePath):
    structs = []
    with open(filePath, 'r', encoding='utf8') as f:
        struct = None
        for line in f:
            match = re.search('struct\s*([A-Za-z0-9_]+)', line)
            if match:
                struct = Struct(match.group(1))
            elif struct is not None:
                if line.startswith('};'): # XXX: kinda fragile
                    if (len(struct.fields)):
                        structs.append(struct)
                    struct = None
                else:
                    parseStructField(line, struct)
    return structs

def fwrite(f, str):
    f.write(str.encode('utf-8'))
    
def emit(structs, filePath):
    maxFieldCount = 0
    with open(filePath, 'wb') as f:
        for struct in structs:
            fwrite(f, 'void serialize(const MetadataManager &mgr, EmbeddedString &string, {} &metaData) {{\n'.format(struct.name))
            for fieldName in struct.fields:
                fwrite(f, '\tserialize(mgr, string, metaData.{});\n'.format(fieldName))
            if len(struct.fields) > maxFieldCount:
                maxFieldCount = len(struct.fields)
            fwrite(f, '}\n')
        
        fwrite(f, 'enum : uint32_t {{ MaxFieldCount = {} }};\n'.format(maxFieldCount))
    
if __name__ == "__main__":
    assert len(sys.argv) == 3
    
    structs = load(sys.argv[1])
    emit(structs, sys.argv[2])
    
        