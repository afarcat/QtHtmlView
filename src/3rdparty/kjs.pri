# Copyright (C) 2018 afarcat <kabak@sina.com>. All rights reserved.
#   Use of this source code is governed by a Apache license that can be
#   found in the LICENSE file.

INCLUDEPATH += $$PWD/kjs/ $$PWD/kjs/src
INCLUDEPATH += $$PWD/kjs/src/kjs $$PWD/kjs/src/wtf

ENABLE_KJS {
# Default location of generated sources
GENERATED_SOURCES_DESTDIR = $$OUT_PWD/generated/kjs
INCLUDEPATH += $${GENERATED_SOURCES_DESTDIR}

#lut
KJS_LUT_SOURCES = \
    $$PWD/kjs/src/kjs/array_object.cpp \
    $$PWD/kjs/src/kjs/date_object.cpp \
    $$PWD/kjs/src/kjs/json_object.cpp \
    $$PWD/kjs/src/kjs/keywords.table \
    $$PWD/kjs/src/kjs/math_object.cpp \
    $$PWD/kjs/src/kjs/number_object.cpp \
    $$PWD/kjs/src/kjs/regexp_object.cpp \
    $$PWD/kjs/src/kjs/string_object.cpp

kjs_lut_files.name = generate .h file for ${QMAKE_FILE_BASE}.cpp
kjs_lut_files.CONFIG = no_link target_predeps
kjs_lut_files.input = KJS_LUT_SOURCES
kjs_lut_files.output = $${GENERATED_SOURCES_DESTDIR}/${QMAKE_FILE_BASE}.lut.h
kjs_lut_files.commands = perl $$PWD/kjs/src/kjs/create_hash_table ${QMAKE_FILE_IN} -i > ${QMAKE_FILE_OUT}
QMAKE_EXTRA_COMPILERS += kjs_lut_files

HEADERS += \
    $$PWD/kjs/src/kjs/array_instance.h \
    $$PWD/kjs/src/kjs/array_object.h \
    $$PWD/kjs/src/kjs/bool_object.h \
    $$PWD/kjs/src/kjs/collector.h \
    $$PWD/kjs/src/kjs/CommonIdentifiers.h \
    $$PWD/kjs/src/kjs/commonunicode.h \
    $$PWD/kjs/src/kjs/CompileState.h \
    $$PWD/kjs/src/kjs/completion.h \
    $$PWD/kjs/src/kjs/context.h \
    $$PWD/kjs/src/kjs/date_object.h \
    $$PWD/kjs/src/kjs/debugger.h \
    $$PWD/kjs/src/kjs/dtoa.h \
    $$PWD/kjs/src/kjs/error_object.h \
    $$PWD/kjs/src/kjs/ExecState.h \
    $$PWD/kjs/src/kjs/function.h \
    $$PWD/kjs/src/kjs/function_object.h \
    $$PWD/kjs/src/kjs/global.h \
    $$PWD/kjs/src/kjs/grammar.h \
    $$PWD/kjs/src/kjs/grammar.y \
    $$PWD/kjs/src/kjs/identifier.h \
    $$PWD/kjs/src/kjs/internal.h \
    $$PWD/kjs/src/kjs/interpreter.h \
    $$PWD/kjs/src/kjs/JSImmediate.h \
    $$PWD/kjs/src/kjs/JSLock.h \
    $$PWD/kjs/src/kjs/json_object.h \
    $$PWD/kjs/src/kjs/jsonlexer.h \
    $$PWD/kjs/src/kjs/jsonstringify.h \
    $$PWD/kjs/src/kjs/JSType.h \
    $$PWD/kjs/src/kjs/JSVariableObject.h \
    $$PWD/kjs/src/kjs/JSWrapperObject.h \
    $$PWD/kjs/src/kjs/lexer.h \
    $$PWD/kjs/src/kjs/list.h \
    $$PWD/kjs/src/kjs/LocalStorage.h \
    $$PWD/kjs/src/kjs/lookup.h \
    $$PWD/kjs/src/kjs/makenodes.h \
    $$PWD/kjs/src/kjs/math_object.h \
    $$PWD/kjs/src/kjs/nodes.h \
    $$PWD/kjs/src/kjs/nodes2bytecode.h \
    $$PWD/kjs/src/kjs/number_object.h \
    $$PWD/kjs/src/kjs/object.h \
    $$PWD/kjs/src/kjs/object_object.h \
    $$PWD/kjs/src/kjs/operations.h \
    $$PWD/kjs/src/kjs/package.h \
    $$PWD/kjs/src/kjs/Parser.h \
    $$PWD/kjs/src/kjs/property_map.h \
    $$PWD/kjs/src/kjs/property_slot.h \
    $$PWD/kjs/src/kjs/propertydescriptor.h \
    $$PWD/kjs/src/kjs/PropertyNameArray.h \
    $$PWD/kjs/src/kjs/protect.h \
    $$PWD/kjs/src/kjs/regexp.h \
    $$PWD/kjs/src/kjs/regexp_object.h \
    $$PWD/kjs/src/kjs/SavedBuiltins.h \
    $$PWD/kjs/src/kjs/scope_chain.h \
    $$PWD/kjs/src/kjs/scriptfunction.h \
    $$PWD/kjs/src/kjs/string_object.h \
    $$PWD/kjs/src/kjs/SymbolTable.h \
    $$PWD/kjs/src/kjs/types.h \
    $$PWD/kjs/src/kjs/ustring.h \
    $$PWD/kjs/src/kjs/value.h

SOURCES += \
    $$PWD/kjs/src/kjs/array_instance.cpp \
    $$PWD/kjs/src/kjs/array_object.cpp \
    $$PWD/kjs/src/kjs/bool_object.cpp \
    $$PWD/kjs/src/kjs/collector.cpp \
    $$PWD/kjs/src/kjs/CommonIdentifiers.cpp \
    $$PWD/kjs/src/kjs/CompileState.cpp \
    $$PWD/kjs/src/kjs/date_object.cpp \
    $$PWD/kjs/src/kjs/debugger.cpp \
    $$PWD/kjs/src/kjs/dtoa.cpp \
    $$PWD/kjs/src/kjs/error_object.cpp \
    $$PWD/kjs/src/kjs/ExecState.cpp \
    $$PWD/kjs/src/kjs/fpconst.cpp \
    $$PWD/kjs/src/kjs/function.cpp \
    $$PWD/kjs/src/kjs/function_object.cpp \
    $$PWD/kjs/src/kjs/grammar.cpp \
    $$PWD/kjs/src/kjs/identifier.cpp \
    $$PWD/kjs/src/kjs/internal.cpp \
    $$PWD/kjs/src/kjs/interpreter.cpp \
    $$PWD/kjs/src/kjs/JSImmediate.cpp \
    $$PWD/kjs/src/kjs/JSLock.cpp \
    $$PWD/kjs/src/kjs/json_object.cpp \
    $$PWD/kjs/src/kjs/jsonlexer.cpp \
    $$PWD/kjs/src/kjs/jsonstringify.cpp \
    $$PWD/kjs/src/kjs/JSVariableObject.cpp \
    $$PWD/kjs/src/kjs/JSWrapperObject.cpp \
    $$PWD/kjs/src/kjs/lexer.cpp \
    $$PWD/kjs/src/kjs/list.cpp \
    $$PWD/kjs/src/kjs/lookup.cpp \
    $$PWD/kjs/src/kjs/math_object.cpp \
    $$PWD/kjs/src/kjs/nodes.cpp \
    $$PWD/kjs/src/kjs/nodes2bytecode.cpp \
    $$PWD/kjs/src/kjs/nodes2string.cpp \
    $$PWD/kjs/src/kjs/number_object.cpp \
    $$PWD/kjs/src/kjs/object.cpp \
    $$PWD/kjs/src/kjs/object_object.cpp \
    $$PWD/kjs/src/kjs/operations.cpp \
    $$PWD/kjs/src/kjs/package.cpp \
    $$PWD/kjs/src/kjs/Parser.cpp \
    $$PWD/kjs/src/kjs/property_map.cpp \
    $$PWD/kjs/src/kjs/property_slot.cpp \
    $$PWD/kjs/src/kjs/propertydescriptor.cpp \
    $$PWD/kjs/src/kjs/PropertyNameArray.cpp \
    $$PWD/kjs/src/kjs/regexp.cpp \
    $$PWD/kjs/src/kjs/regexp_object.cpp \
    $$PWD/kjs/src/kjs/scope_chain.cpp \
    $$PWD/kjs/src/kjs/string_object.cpp \
    $$PWD/kjs/src/kjs/ustring.cpp \
    $$PWD/kjs/src/kjs/value.cpp \
    $$PWD/kjs/src/kjs/bytecode/machine.cpp \
    $$PWD/kjs/src/kjs/bytecode/opcodes.cpp
}
