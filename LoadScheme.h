#pragma once

#include <tll/scheme.h>

static tll::scheme::SchemePtr
LoadScheme() {
    tll::scheme::SchemePtr s(tll::Scheme::load(R"(yamls://
- name: ''
  enums:
    my_enum: {type: int8, enum: {FIRST: 1, SECOND: 2} }

- name: empty_message
  id: 1

- name: my_message
  id: 2
  fields:
    - { name: enum_field, type: my_enum }
    - { name: string_field, type: byte10, options: {type: string}}
    - { name: time_point_field, type: uint64, options: {type: 'time_point', resolution: 'ns'} }
    - { name: duration_field, type: int32, options: {type: 'duration', resolution: 'ms'}}
    - { name: fixed_field, type: int64, options: {type: 'fixed8'}}
    - { name: bits_field, type: int8, options.type: bits, bits: [a, b, c]}
    - { name: decimal128_field, type: decimal128 }
    - { name: bytes_field, type: byte3 }
    - { name: message_field, type: empty_message}
    - { name: array_field, type: 'int16[5]', list-options.count-type: int64}
    - { name: pointer_field, type: '*int16'}
    - { name: union_field, type: union, union: [{name: i8, type: int8}, {name: d, type: double}, {name: m, type: empty_message}]}
    - { name: primitive_field, type: double}
)"));

    return s;
}