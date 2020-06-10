#! /usr/bin/env python

import datetime, argparse, os
from collections import OrderedDict

# Generators must be kept in sync with the relevant struct definitions/functions inside the Engine/Compiler

def capwords(str):
    return ''.join(word.capitalize() for word in str.split('-'))

class Property(object):
    """Class that represents all the info we have about a css property

    the objects have following properties:
    - name         css property name ('lowercase-with-dashes')
    - Name         name in camelcase ('LowercaseWithDashes')
    - enum         Name prepended with 'eSldCSSProperty_' ('eSldCSSProperty_LowercaseWithDashes')
    - enum_values  string representing the 'CamelCase' part of the enum if the prop has enum values
    - multi_value  flag if the property can have multiple values
    - value_types  list of supported value types in string lowercase form
    - value_flags  list of supported value flags in string lowercase form
    """
    def __init__(self, name, value_types, enum_values, multi_value, value_flags):
        """Parameters:

        - name is a css property name (lowercase-with-dashes)
        - value_types is a string of supported value types (enums are implictly handled if enum_values is not empty)
          `initial` and `inherit` are supported on all css values
        - enum_values is a string representing the "CamelCase" part of the enum if the prop has enum values
        - multi_value is a flag if the property can have multiple values
        """
        assert isinstance(name, str)
        assert isinstance(value_types, str)
        assert isinstance(enum_values, str)
        assert isinstance(multi_value, bool)
        assert isinstance(value_flags, str)

        self.name = name
        self.Name = capwords(name)
        self.enum = 'eSldCSSProperty_' + self.Name
        self.multi_value = multi_value
        self.enum_values = enum_values
        self.value_types = value_types.split()
        self.value_flags = value_flags.split()

        if self.enum_values:
            self.value_types.append('enum')

    def __lt__(self, other):
        """ sorted() support """
        return self.name < other.name

sides = [ 'top', 'right', 'bottom', 'left' ]

class FourSidedProperty(object):
    """Class that represents all the info we have about an 'umbrella' css prop defining 4 sides at once

    the objects have following properties:
    - fmt   'template' name for a property, contains '{side}' ('lowercase-{side}-dashes')
    - name  property name without '{side}' ('lowercase-dashes')
    - Name  name in camelcase ('LowercaseDashes')
    """
    def __init__(self, name):
        assert isinstance(name, str)
        assert '{side}' in name

        self.fmt = name
        self.name = name.replace('{side}', '').replace('--', '-').strip('-')
        self.Name = capwords(self.name)

    def side_name(self, side):
        return self.fmt.format(side = side)

    def sideName(self, side):
        return capwords(self.side_name(side))

    # the following properties are here only as formatting helpers
    @property
    def sideNames(self):
        return [self.sideName(side) for side in sides]

    @property
    def enum(self):
        return self.name

    def __lt__(self, other):
        """ sorted() support """
        return self.name < other.name

# generic value types
none       = 'none '
auto       = 'auto '
number     = 'number '
length     = 'length '
percentage = 'percentage '
color      = 'color '
url        = 'url '
string     = 'string '
# custom value types
sld_image  = 'sld-image-url '
sld_string = 'sld-string-literal '
# value flags
nonneg     = 'non-negative '
intonly    = 'int-only '

# global list of properties
css_props = OrderedDict()
four_sided_props = {}

# PropertyName -> supported enum values list
css_enums = {}

def _property(name, value_types, enum_values, multi_value, value_flags):
    assert name not in css_props
    css_props[name] = Property(name, value_types, enum_values, multi_value, value_flags)

def cssprop(name, value_types, enum_values = '', multi_value = False, value_flags = ''):
    if enum_values:
        enumName = capwords(name)
        assert enumName not in css_enums
        css_enums[enumName] = enum_values.split('|')
        enum_values = enumName

    _property(name, value_types, enum_values, multi_value, value_flags)

def cssprop4sided(name, value_types, enum_values = '', multi_value = False, value_flags = ''):
    prop = FourSidedProperty(name)
    assert prop.name not in css_props
    four_sided_props[prop.name] = prop

    # add the "shared" enum to the enums list
    if enum_values:
        assert prop.Name not in css_enums
        css_enums[prop.Name] = enum_values.split('|')
        enum_values = prop.Name

    for side in sides:
        _property(prop.side_name(side), value_types, enum_values, multi_value, value_flags)

# properties
# new ones should be added to the end of the list (the order matters)
# new css property enums values also should be added at the end of the respective string
cssprop('display', none, 'inline|block|inline-block|flex|inline-flex')

cssprop4sided('margin-{side}', auto + length + percentage)

cssprop('width',  auto + length + percentage, value_flags = nonneg)
cssprop('height', auto + length + percentage, value_flags = nonneg)
cssprop('float', none, 'left|right')
cssprop('clear', none, 'left|right|both')

cssprop4sided('padding-{side}', length + percentage, value_flags = nonneg)

cssprop('background-image', none + sld_image)
cssprop('background-repeat', '', 'repeat|repeat-x|repeat-y|no-repeat')
cssprop('background-size', auto + length + percentage, 'cover|contain', multi_value = True)
cssprop('background-position', length + percentage, 'left|right|top|center|bottom', multi_value = True)
cssprop('text-align', '', 'left|right|center|justify')

cssprop4sided('border-{side}-width', length, 'medium|thin|thick', value_flags = nonneg)
cssprop4sided('border-{side}-style', none, 'hidden|dotted|dashed|solid|double|groove|ridge|inset|outset')
cssprop4sided('border-{side}-color', color)

cssprop('vertical-align', length + percentage, 'baseline|sub|super|top|text-top|middle|bottom|text-bottom')

cssprop('background-color', color)

# border-*-radius works with corners instead of sides so we have to hack around a bit
class BorderRadius(FourSidedProperty):
    corners = [ 'top-left', 'top-right', 'bottom-right', 'bottom-left' ]
    
    def __init__(self):
        super(BorderRadius, self).__init__('border-{side}-radius')

    @property
    def sideNames(self):
        return [self.sideName(corner) for corner in self.corners]

for corner in BorderRadius.corners:
    cssprop('border-' + corner + '-radius', length + percentage, value_flags = nonneg)
four_sided_props['border-radius'] = BorderRadius()

cssprop('position', '', 'static|relative|absolute|fixed')
for side in sides: # sides
    cssprop(side, auto + length + percentage)
cssprop('z-index', auto + number, value_flags = intonly)

cssprop('justify-content', '', 'flex-start|flex-end|center|space-between|space-around')
cssprop('align-items', '', 'flex-start|flex-end|center|baseline|stretch')

cssprop('min-height', length + percentage, value_flags = nonneg)
cssprop('max-height', none + length + percentage, value_flags = nonneg)
cssprop('min-width', length + percentage, value_flags = nonneg)
cssprop('max-width', none + length + percentage, value_flags = nonneg)

cssprop('text-indent', length + percentage)

cssprop('direction', '', 'ltr|rtl')

# Code generation code below
# Must be kept in sync with the relevant datastructures inside the enigne/compiler

class PropFormatter(object):
    def __init__(self, prop, proxy = None):
        assert isinstance(prop, Property)
        self._prop = prop
        self._proxy = proxy

    @property
    def enums_count(self):
        return len(css_enums[self._prop.enum_values]) if self._prop.enum_values else 0

    @property
    def enum_values(self):
        return '{}_enums'.format(self._prop.enum_values) if self._prop.enum_values else 'NULL'

    def __getattr__(self, key):
        if self._proxy is None:
            return getattr(self._prop, key)

        try:
            return getattr(self._proxy, key)(self._prop)
        except AttributeError:
            return getattr(self._prop, key)


def format_file_header(blurb):
    return '''// {blurb}
// Autogenerated on {date}
// Do *NOT* hand edit! Any modifications *SHOULD* be done through the generator script
'''.format(date = datetime.datetime.now(), blurb = blurb)

# Generates ESldCSSPropertyType definition file for the engine
def generate_engine_defines():
    out = format_file_header('CSS property type enum definintion')

    out += '''
enum ESldCSSPropertyType {{
	eSldCSSPropertyType_Invalid = 0,

{},

	eSldCSSPropertyType_Count,
	eSldCSSPropertyType_Last = eSldCSSPropertyType_Count - 1
}};
'''.format(',\n'.join('\t{prop.enum}'.format(prop = p) for p in css_props.values()))

    return out

# Generates struct definitions file for the engine
def generate_engine_structs():
    out = format_file_header('CSS properties defintions structs')

    for name, values in sorted(css_enums.items()):
        out += '''
static const UInt16 *const {0}_enums[] = {{
	{1}
}};
'''.format(name, ',\n\t'.join('SldU16StringLiteral("{}")'.format(e) for e in values))

    for prop in sorted(css_props.values()):
        out +='''
static const PropertyDescriptor {prop.Name} = {{
	/* .name = */ SldU16StringLiteral("{prop.name}"),
	/* .enumCount = */ {prop.enums_count},
	/* .enums = */ {prop.enum_values}
}};
'''.format(prop = PropFormatter(prop))

    out += '''
static const PropertyDescriptor *const descriptors[] = {{
	/* [eSldCSSPropertyType_Invalid] = */ NULL,

{}
}};
'''.format(',\n'.join('\t/* [{prop.enum}] = */ &{prop.Name}'.format(prop = p) for p in css_props.values()))

    return out


def emit(str, filepath):
    with open(filepath, 'wb') as f:
        f.write(str.encode('utf-8'))


if __name__ == "__main__":
    engine_structs_filepath = './Engine/SldCSSProperties.incl'
    engine_defines_filepath = './Engine/SldCSSPropertyType.h'

    parser = argparse.ArgumentParser(description='CSS property code generator')
    parser.add_argument('target', choices=['structs', 'defs', 'all'])
    parser.add_argument('-o', type=str, metavar='FILE', help = 'output file path [has no effect for `all` target]')
    opts = parser.parse_args()
    if opts.target == 'all':
        emit(generate_engine_structs(), engine_structs_filepath)
        emit(generate_engine_defines(), engine_defines_filepath)
    elif opts.target == 'structs':
        emit(generate_engine_structs(), opts.o if opts.o else engine_structs_filepath)
    elif opts.target == 'defs':
        emit(generate_engine_defines(), opts.o if opts.o else engine_defines_filepath)


