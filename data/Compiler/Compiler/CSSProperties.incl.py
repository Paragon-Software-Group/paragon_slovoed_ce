#! /usr/bin/env python

import sys

def capwords(str):
    return ''.join(word.capitalize() for word in str.split('-'))

class PropFormatter(object):
    def __init__(self, prop):
        self._prop = prop

    @property
    def enums_count(self):
        return len(css_enums[self._prop.enum_values]) if self._prop.enum_values else 0

    @property
    def enum_values(self):
        return '{}_enums'.format(self._prop.enum_values) if self._prop.enum_values else 'nullptr'

    @property
    def multi_value(self):
        return 'true' if self._prop.multi_value else 'false'

    @property
    def valid_values_mask(self):
        if len(self._prop.value_types) == 0:
            return '0'
        return ' | '.join('VMASK({})'.format(capwords(v)) for v in self._prop.value_types)

    @property
    def valid_values_flags(self):
        if len(self._prop.value_flags) == 0:
            return '0'
        return ' | '.join('FMASK({})'.format(capwords(v)) for v in self._prop.value_flags)

    def __getattr__(self, key):
        return getattr(self._prop, key)


def format_descriptor_table(values):
    return ',\n'.join('\t/* [{prop.enum}] = */ &{prop.Name}'.format(prop = p) for p in values)
        
# Generates struct definitions file for the compiler
def generate():
    out = ''
    for name, values in sorted(css_enums.items()):
        out += '''
static const wchar_t *const {0}_enums[] = {{ {1} }};
'''.format(name, ', '.join('L"{}"'.format(e) for e in values))

    for prop in sorted(css_props.values()):
        out += '''
static const PropertyDescriptor {prop.Name} = {{
	/* .name = */ L"{prop.name}",
	/* .type = */ {prop.enum},
	/* .validValuesMask = */ {prop.valid_values_mask},
	/* .validValuesFlags = */ {prop.valid_values_flags},
	/* .multiValue = */ {prop.multi_value},
	/* .enumCount = */ {prop.enums_count},
	/* .enums = */ {prop.enum_values}
}};
'''.format(prop = PropFormatter(prop))

    for prop in sorted(four_sided_props.values()):
        out += '''
static const FourSidedPropertyDescriptor {prop.Name} = {{
	/* .name = */ L"{prop.name}",
	/* .sides = */ {{ {sides} }}
}};
'''.format(prop = prop, sides = ', '.join('&{}'.format(s) for s in prop.sideNames))

    if four_sided_props:
        out += '''
static const FourSidedPropertyDescriptor *const fourSidedProps[] = {{
{}
}};
'''.format(format_descriptor_table(sorted(four_sided_props.values())))

    out += '''
static const PropertyDescriptor *const simpleProps[] = {{
	/* [eSldCSSPropertyType_Invalid] = */ nullptr,

{}
}};
'''.format(format_descriptor_table(css_props.values()))

    return out

if __name__ == "__main__":

    # import css related data
    sys.path.append(sys.argv[1])
    from sld_css_props import css_props, four_sided_props, css_enums
    
    with open(sys.argv[2], 'wb') as f:
        f.write(generate().encode('utf-8'))
