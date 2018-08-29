# Copyright (C) 2018 afarcat <kabak@sina.com>. All rights reserved.
#   Use of this source code is governed by a Apache license that can be
#   found in the LICENSE file.

INCLUDEPATH += $$PWD/khtml/src
INCLUDEPATH += $$PWD/khtml/src/dom $$PWD/khtml/src/html
INCLUDEPATH += $$PWD/khtml/src/imload $$PWD/khtml/src/imload/decoders
INCLUDEPATH += $$PWD/khtml/src/misc $$PWD/khtml/src/xml
INCLUDEPATH += $$PWD/khtml/src/platform/graphics
INCLUDEPATH += $$PWD/khtml/src/rendering

#css
HEADERS += \
    $$PWD/khtml/src/css/css_base.h \
    $$PWD/khtml/src/css/css_mediaquery.h \
    $$PWD/khtml/src/css/css_renderstyledeclarationimpl.h \
    $$PWD/khtml/src/css/css_ruleimpl.h \
    $$PWD/khtml/src/css/css_stylesheetimpl.h \
    $$PWD/khtml/src/css/css_valueimpl.h \
    $$PWD/khtml/src/css/css_webfont.h \
    $$PWD/khtml/src/css/csshelper.h \
    $$PWD/khtml/src/css/cssparser.h \
    $$PWD/khtml/src/css/cssstyleselector.h \
    $$PWD/khtml/src/css/parser.h \
    #$$PWD/khtml/src/css/css_svgcssparser.h \
    #$$PWD/khtml/src/css/css_svgvalueimpl.h \
    #$$PWD/khtml/src/css/SVGCSSStyleSelector.h

SOURCES += \
    $$PWD/khtml/src/css/css_base.cpp \
    $$PWD/khtml/src/css/css_mediaquery.cpp \
    $$PWD/khtml/src/css/css_renderstyledeclarationimpl.cpp \
    $$PWD/khtml/src/css/css_ruleimpl.cpp \
    $$PWD/khtml/src/css/css_stylesheetimpl.cpp \
    $$PWD/khtml/src/css/css_valueimpl.cpp \
    $$PWD/khtml/src/css/css_webfont.cpp \
    $$PWD/khtml/src/css/csshelper.cpp \
    $$PWD/khtml/src/css/cssparser.cpp \
    $$PWD/khtml/src/css/cssstyleselector.cpp \
    $$PWD/khtml/src/css/parser.cpp \
    #$$PWD/khtml/src/css/css_svgcssparser.cpp \
    #$$PWD/khtml/src/css/css_svgvalueimpl.cpp \
    #$$PWD/khtml/src/css/SVGCSSStyleSelector.cpp

#dom
HEADERS += \
    $$PWD/khtml/src/dom/css_rule.h \
    $$PWD/khtml/src/dom/css_stylesheet.h \
    $$PWD/khtml/src/dom/css_value.h \
    $$PWD/khtml/src/dom/dom_doc.h \
    $$PWD/khtml/src/dom/dom_element.h \
    $$PWD/khtml/src/dom/dom_misc.h \
    $$PWD/khtml/src/dom/dom_node.h \
    $$PWD/khtml/src/dom/dom_string.h \
    $$PWD/khtml/src/dom/dom_text.h \
    $$PWD/khtml/src/dom/dom_xml.h \
    $$PWD/khtml/src/dom/dom2_events.h \
    $$PWD/khtml/src/dom/dom2_range.h \
    $$PWD/khtml/src/dom/dom2_traversal.h \
    $$PWD/khtml/src/dom/dom2_views.h \
    $$PWD/khtml/src/dom/dom3_xpath.h \
    $$PWD/khtml/src/dom/html_base.h \
    $$PWD/khtml/src/dom/html_block.h \
    $$PWD/khtml/src/dom/html_document.h \
    $$PWD/khtml/src/dom/html_element.h \
    $$PWD/khtml/src/dom/html_form.h \
    $$PWD/khtml/src/dom/html_head.h \
    $$PWD/khtml/src/dom/html_image.h \
    $$PWD/khtml/src/dom/html_inline.h \
    $$PWD/khtml/src/dom/html_list.h \
    $$PWD/khtml/src/dom/html_misc.h \
    $$PWD/khtml/src/dom/html_object.h \
    $$PWD/khtml/src/dom/html_table.h \
    $$PWD/khtml/src/dom/QualifiedName.h

SOURCES += \
    $$PWD/khtml/src/dom/dom_misc.cpp \
    $$PWD/khtml/src/dom/dom_string.cpp \
    $$PWD/khtml/src/dom/QualifiedName.cpp

ENABLE_DOM {
SOURCES += \
    $$PWD/khtml/src/dom/css_rule.cpp \
    $$PWD/khtml/src/dom/css_stylesheet.cpp \
    $$PWD/khtml/src/dom/css_value.cpp \
    $$PWD/khtml/src/dom/dom_doc.cpp \
    $$PWD/khtml/src/dom/dom_element.cpp \
    $$PWD/khtml/src/dom/dom_node.cpp \
    $$PWD/khtml/src/dom/dom_text.cpp \
    $$PWD/khtml/src/dom/dom_xml.cpp \
    $$PWD/khtml/src/dom/dom2_events.cpp \
    $$PWD/khtml/src/dom/dom2_range.cpp \
    $$PWD/khtml/src/dom/dom2_views.cpp \
    $$PWD/khtml/src/dom/html_base.cpp \
    $$PWD/khtml/src/dom/html_block.cpp \
    $$PWD/khtml/src/dom/html_document.cpp \
    $$PWD/khtml/src/dom/html_element.cpp \
    $$PWD/khtml/src/dom/html_form.cpp \
    $$PWD/khtml/src/dom/html_head.cpp \
    $$PWD/khtml/src/dom/html_image.cpp \
    $$PWD/khtml/src/dom/html_inline.cpp \
    $$PWD/khtml/src/dom/html_list.cpp \
    $$PWD/khtml/src/dom/html_misc.cpp \
    $$PWD/khtml/src/dom/html_object.cpp \
    $$PWD/khtml/src/dom/html_table.cpp
}

#ecma
ENABLE_ECMA {
# Default location of generated sources
GENERATED_SOURCES_DESTDIR = $$OUT_PWD/generated/ecma
INCLUDEPATH += $${GENERATED_SOURCES_DESTDIR}
qtPrepareTool(QMAKE_MOC, moc)

#lut
ECMA_LUT_SOURCES = \
    $$PWD/khtml/src/ecma/domparser.cpp \
    $$PWD/khtml/src/ecma/kjs_arraybuffer.cpp \
    $$PWD/khtml/src/ecma/kjs_arraybufferview.cpp \
    $$PWD/khtml/src/ecma/kjs_clientrect.cpp \
    $$PWD/khtml/src/ecma/kjs_context2d.cpp \
    $$PWD/khtml/src/ecma/kjs_css.cpp \
    $$PWD/khtml/src/ecma/kjs_dom.cpp \
    $$PWD/khtml/src/ecma/kjs_events.cpp \
    $$PWD/khtml/src/ecma/kjs_html.cpp \
    $$PWD/khtml/src/ecma/kjs_mozilla.cpp \
    $$PWD/khtml/src/ecma/kjs_navigator.cpp \
    $$PWD/khtml/src/ecma/kjs_range.cpp \
    $$PWD/khtml/src/ecma/kjs_traversal.cpp \
    $$PWD/khtml/src/ecma/kjs_views.cpp \
    $$PWD/khtml/src/ecma/kjs_window.cpp \
    $$PWD/khtml/src/ecma/kjs_xpath.cpp \
    $$PWD/khtml/src/ecma/xmlhttprequest.cpp \
    $$PWD/khtml/src/ecma/xmlserializer.cpp

ecma_lut_files.name = generate .h file for ${QMAKE_FILE_BASE}.cpp
ecma_lut_files.CONFIG = no_link target_predeps
ecma_lut_files.input = ECMA_LUT_SOURCES
ecma_lut_files.output = $${GENERATED_SOURCES_DESTDIR}/${QMAKE_FILE_BASE}.lut.h
ecma_lut_files.commands = perl $$PWD/kjs/src/kjs/create_hash_table ${QMAKE_FILE_IN} > ${QMAKE_FILE_OUT}
QMAKE_EXTRA_COMPILERS += ecma_lut_files

#ECMA_JS_BINDING = \
#    $$PWD/khtml/src/html/HTMLAudioElement.idl \
#    $$PWD/khtml/src/html/HTMLMediaElement.idl \
#    $$PWD/khtml/src/html/HTMLVideoElement.idl \
#    $$PWD/khtml/src/html/MediaError.idl \
#    $$PWD/khtml/src/html/TimeRanges.idl

#ecma_js_binding_files_h.CONFIG = no_link target_predeps
#ecma_js_binding_files_h.input = ECMA_JS_BINDING
#ecma_js_binding_files_h.output = $${GENERATED_SOURCES_DESTDIR}/${QMAKE_FILE_BASE}.h
#ecma_js_binding_files_h.commands = perl -I$$PWD/khtml/src/bindings/scripts $$PWD/khtml/src/bindings/scripts/generate-bindings.pl --include=$$PWD/khtml/src/html --generator=JS --preprocessor=\"$${QMAKE_MOC} -E\" --defines=ENABLE_VIDEO ${QMAKE_FILE_IN} --outputdir ${QMAKE_FILE_OUT}
#QMAKE_EXTRA_COMPILERS += ecma_js_binding_files_h

HEADERS += \
    $$PWD/khtml/src/ecma/domparser.h \
    $$PWD/khtml/src/ecma/JSHTMLElement.h \
    $$PWD/khtml/src/ecma/kjs_arraybuffer.h \
    $$PWD/khtml/src/ecma/kjs_arraybufferview.h \
    $$PWD/khtml/src/ecma/kjs_arraytyped.h \
    $$PWD/khtml/src/ecma/kjs_audio.h \
    $$PWD/khtml/src/ecma/kjs_binding.h \
    $$PWD/khtml/src/ecma/kjs_clientrect.h \
    $$PWD/khtml/src/ecma/kjs_context2d.h \
    $$PWD/khtml/src/ecma/kjs_css.h \
    $$PWD/khtml/src/ecma/kjs_data.h \
    $$PWD/khtml/src/ecma/kjs_dom.h \
    $$PWD/khtml/src/ecma/kjs_events.h \
    $$PWD/khtml/src/ecma/kjs_html.h \
    #$$PWD/khtml/src/ecma/kjs_mozilla.h \
    #$$PWD/khtml/src/ecma/kjs_navigator.h \
    $$PWD/khtml/src/ecma/kjs_proxy.h \
    $$PWD/khtml/src/ecma/kjs_range.h \
    $$PWD/khtml/src/ecma/kjs_scriptable.h \
    $$PWD/khtml/src/ecma/kjs_traversal.h \
    $$PWD/khtml/src/ecma/kjs_views.h \
    $$PWD/khtml/src/ecma/kjs_window.h \
    $$PWD/khtml/src/ecma/kjs_xpath.h \
    $$PWD/khtml/src/ecma/xmlhttprequest.h \
    $$PWD/khtml/src/ecma/xmlserializer.h

SOURCES += \
    $$PWD/khtml/src/ecma/domparser.cpp \
    $$PWD/khtml/src/ecma/JSHTMLElement.cpp \
    $$PWD/khtml/src/ecma/kjs_arraybuffer.cpp \
    $$PWD/khtml/src/ecma/kjs_arraybufferview.cpp \
    $$PWD/khtml/src/ecma/kjs_arraytyped.cpp \
    $$PWD/khtml/src/ecma/kjs_audio.cpp \
    $$PWD/khtml/src/ecma/kjs_binding.cpp \
    $$PWD/khtml/src/ecma/kjs_clientrect.cpp \
    $$PWD/khtml/src/ecma/kjs_context2d.cpp \
    $$PWD/khtml/src/ecma/kjs_css.cpp \
    $$PWD/khtml/src/ecma/kjs_data.cpp \
    $$PWD/khtml/src/ecma/kjs_dom.cpp \
    $$PWD/khtml/src/ecma/kjs_events.cpp \
    $$PWD/khtml/src/ecma/kjs_html.cpp \
    #$$PWD/khtml/src/ecma/kjs_mozilla.cpp \
    #$$PWD/khtml/src/ecma/kjs_navigator.cpp \
    $$PWD/khtml/src/ecma/kjs_proxy.cpp \
    $$PWD/khtml/src/ecma/kjs_range.cpp \
    $$PWD/khtml/src/ecma/kjs_scriptable.cpp \
    $$PWD/khtml/src/ecma/kjs_traversal.cpp \
    $$PWD/khtml/src/ecma/kjs_views.cpp \
    $$PWD/khtml/src/ecma/kjs_window.cpp \
    $$PWD/khtml/src/ecma/kjs_xpath.cpp \
    $$PWD/khtml/src/ecma/xmlhttprequest.cpp \
    $$PWD/khtml/src/ecma/xmlserializer.cpp
}

#editing
HEADERS += \
    $$PWD/khtml/src/editing/editing_p.h \
    $$PWD/khtml/src/editing/editor.h \
    $$PWD/khtml/src/editing/htmlediting_impl.h \
    $$PWD/khtml/src/editing/jsediting.h

SOURCES += \
    $$PWD/khtml/src/editing/editing.cpp \
    $$PWD/khtml/src/editing/editor.cpp \
    $$PWD/khtml/src/editing/htmlediting_impl.cpp \
    $$PWD/khtml/src/editing/jsediting.cpp

#html
# Default location of generated sources
GENERATED_SOURCES_DESTDIR = $$OUT_PWD/generated/html
INCLUDEPATH += $${GENERATED_SOURCES_DESTDIR}

DOCTYPES_GPERF_SOURCES = \
    $$PWD/khtml/src/html/doctypes.gperf

doctypes_gperf.name = generate .h file for ${QMAKE_FILE_BASE}.cpp
doctypes_gperf.CONFIG = no_link target_predeps
doctypes_gperf.input = DOCTYPES_GPERF_SOURCES
doctypes_gperf.output = $${GENERATED_SOURCES_DESTDIR}/${QMAKE_FILE_BASE}.h
doctypes_gperf.commands = gperf --key-positions=* ${QMAKE_FILE_IN} --output-file=${QMAKE_FILE_OUT}
QMAKE_EXTRA_COMPILERS += doctypes_gperf

KENTITIES_GPERF_SOURCES = \
    $$PWD/khtml/src/html/kentities.gperf

kentities_gperf.name = generate .h file for ${QMAKE_FILE_BASE}.cpp
kentities_gperf.CONFIG = no_link target_predeps
kentities_gperf.input = KENTITIES_GPERF_SOURCES
kentities_gperf.output = $${GENERATED_SOURCES_DESTDIR}/${QMAKE_FILE_BASE}-gperf.h
kentities_gperf.commands = gperf --key-positions=* -D -s 2 ${QMAKE_FILE_IN} --output-file=${QMAKE_FILE_OUT}
QMAKE_EXTRA_COMPILERS += kentities_gperf

HEADERS += \
    $$PWD/khtml/src/html/dtd.h \
    $$PWD/khtml/src/html/html_baseimpl.h \
    $$PWD/khtml/src/html/html_blockimpl.h \
    $$PWD/khtml/src/html/html_canvasimpl.h \
    $$PWD/khtml/src/html/html_documentimpl.h \
    $$PWD/khtml/src/html/html_elementimpl.h \
    $$PWD/khtml/src/html/html_formimpl.h \
    $$PWD/khtml/src/html/html_headimpl.h \
    $$PWD/khtml/src/html/html_imageimpl.h \
    $$PWD/khtml/src/html/html_inlineimpl.h \
    $$PWD/khtml/src/html/html_listimpl.h \
    $$PWD/khtml/src/html/html_miscimpl.h \
    $$PWD/khtml/src/html/html_objectimpl.h \
    $$PWD/khtml/src/html/html_tableimpl.h \
    $$PWD/khtml/src/html/HTMLAudioElement.h \
    $$PWD/khtml/src/html/HTMLMediaElement.h \
    $$PWD/khtml/src/html/htmlparser.h \
    $$PWD/khtml/src/html/htmlprospectivetokenizer.h \
    $$PWD/khtml/src/html/HTMLSourceElement.h \
    $$PWD/khtml/src/html/htmltokenizer.h \
    $$PWD/khtml/src/html/HTMLVideoElement.h \
    $$PWD/khtml/src/html/kentities_p.h \
    $$PWD/khtml/src/html/TimeRanges.h

SOURCES += \
    $$PWD/khtml/src/html/dtd.cpp \
    $$PWD/khtml/src/html/html_baseimpl.cpp \
    $$PWD/khtml/src/html/html_blockimpl.cpp \
    $$PWD/khtml/src/html/html_canvasimpl.cpp \
    $$PWD/khtml/src/html/html_documentimpl.cpp \
    $$PWD/khtml/src/html/html_elementimpl.cpp \
    $$PWD/khtml/src/html/html_formimpl.cpp \
    $$PWD/khtml/src/html/html_headimpl.cpp \
    $$PWD/khtml/src/html/html_imageimpl.cpp \
    $$PWD/khtml/src/html/html_inlineimpl.cpp \
    $$PWD/khtml/src/html/html_listimpl.cpp \
    $$PWD/khtml/src/html/html_miscimpl.cpp \
    $$PWD/khtml/src/html/html_objectimpl.cpp \
    $$PWD/khtml/src/html/html_tableimpl.cpp \
    $$PWD/khtml/src/html/HTMLAudioElement.cpp \
    $$PWD/khtml/src/html/HTMLMediaElement.cpp \
    $$PWD/khtml/src/html/htmlparser.cpp \
    $$PWD/khtml/src/html/htmlprospectivetokenizer.cpp \
    $$PWD/khtml/src/html/HTMLSourceElement.cpp \
    $$PWD/khtml/src/html/htmltokenizer.cpp \
    $$PWD/khtml/src/html/HTMLVideoElement.cpp \
    $$PWD/khtml/src/html/kentities.cpp \
    $$PWD/khtml/src/html/TimeRanges.cpp

#imload
HEADERS += \
    $$PWD/khtml/src/imload/animprovider.h \
    $$PWD/khtml/src/imload/animtimer.h \
    $$PWD/khtml/src/imload/canvasimage.h \
    $$PWD/khtml/src/imload/image.h \
    $$PWD/khtml/src/imload/imagemanager.h \
    $$PWD/khtml/src/imload/imagepainter.h \
    $$PWD/khtml/src/imload/imageplane.h \
    $$PWD/khtml/src/imload/pixmapplane.h \
    $$PWD/khtml/src/imload/rawimageplane.h \
    $$PWD/khtml/src/imload/scaledimageplane.h \
    $$PWD/khtml/src/imload/updater.h \
    $$PWD/khtml/src/imload/decoders/qimageioloader.h \
    $$PWD/khtml/src/imload/decoders/qmoiveloader.h

SOURCES += \
    $$PWD/khtml/src/imload/animprovider.cpp \
    $$PWD/khtml/src/imload/animtimer.cpp \
    $$PWD/khtml/src/imload/canvasimage.cpp \
    $$PWD/khtml/src/imload/image.cpp \
    $$PWD/khtml/src/imload/imagemanager.cpp \
    $$PWD/khtml/src/imload/imagepainter.cpp \
    $$PWD/khtml/src/imload/imageplane.cpp \
    $$PWD/khtml/src/imload/pixmapplane.cpp \
    $$PWD/khtml/src/imload/rawimageplane.cpp \
    $$PWD/khtml/src/imload/scaledimageplane.cpp \
    $$PWD/khtml/src/imload/updater.cpp \
    $$PWD/khtml/src/imload/decoders/qimageioloader.cpp \
    $$PWD/khtml/src/imload/decoders/qmoiveloader.cpp

#misc
HEADERS += \
    $$PWD/khtml/src/misc/arena.h \
    $$PWD/khtml/src/misc/AtomicString.h \
    $$PWD/khtml/src/misc/borderarcstroker.h \
    $$PWD/khtml/src/misc/guess_ja_p.h \
    $$PWD/khtml/src/misc/helper.h \
    $$PWD/khtml/src/misc/htmlnames.h \
    $$PWD/khtml/src/misc/idstring.h \
    $$PWD/khtml/src/misc/imagefilter.h \
    $$PWD/khtml/src/misc/kencodingdetector.h \
    $$PWD/khtml/src/misc/loader.h \
    $$PWD/khtml/src/misc/paintbuffer.h \
    $$PWD/khtml/src/misc/stringit.h \
    $$PWD/khtml/src/misc/woff.h

SOURCES += \
    $$PWD/khtml/src/misc/arena.cpp \
    $$PWD/khtml/src/misc/AtomicString.cpp \
    $$PWD/khtml/src/misc/blocked_icon.cpp \
    $$PWD/khtml/src/misc/borderarcstroker.cpp \
    $$PWD/khtml/src/misc/guess_ja.cpp \
    $$PWD/khtml/src/misc/helper.cpp \
    $$PWD/khtml/src/misc/htmlnames.cpp \
    $$PWD/khtml/src/misc/idstring.cpp \
    $$PWD/khtml/src/misc/imagefilter.cpp \
    $$PWD/khtml/src/misc/kencodingdetector.cpp \
    $$PWD/khtml/src/misc/loader.cpp \
    $$PWD/khtml/src/misc/paintbuffer.cpp \
    $$PWD/khtml/src/misc/stringit.cpp

ENABLE_WOFF {
SOURCES += \
    $$PWD/khtml/src/misc/woff.cpp
}

#rendering
HEADERS += \
    $$PWD/khtml/src/rendering/bidi.h \
    $$PWD/khtml/src/rendering/break_lines.h \
    $$PWD/khtml/src/rendering/counter_tree.h \
    $$PWD/khtml/src/rendering/enumerate.h \
    $$PWD/khtml/src/rendering/font.h \
    $$PWD/khtml/src/rendering/media_controls.h \
    $$PWD/khtml/src/rendering/render_arena.h \
    $$PWD/khtml/src/rendering/render_block.h \
    $$PWD/khtml/src/rendering/render_body.h \
    $$PWD/khtml/src/rendering/render_box.h \
    $$PWD/khtml/src/rendering/render_br.h \
    $$PWD/khtml/src/rendering/render_canvas.h \
    $$PWD/khtml/src/rendering/render_canvasimage.h \
    $$PWD/khtml/src/rendering/render_container.h \
    $$PWD/khtml/src/rendering/render_flow.h \
    $$PWD/khtml/src/rendering/render_form.h \
    $$PWD/khtml/src/rendering/render_frames.h \
    $$PWD/khtml/src/rendering/render_generated.h \
    $$PWD/khtml/src/rendering/render_image.h \
    $$PWD/khtml/src/rendering/render_inline.h \
    $$PWD/khtml/src/rendering/render_layer.h \
    $$PWD/khtml/src/rendering/render_line.h \
    $$PWD/khtml/src/rendering/render_list.h \
    $$PWD/khtml/src/rendering/render_media.h \
    $$PWD/khtml/src/rendering/render_object.h \
    $$PWD/khtml/src/rendering/render_position.h \
    $$PWD/khtml/src/rendering/render_replaced.h \
    $$PWD/khtml/src/rendering/render_style.h \
    $$PWD/khtml/src/rendering/render_table.h \
    $$PWD/khtml/src/rendering/render_text.h \
    $$PWD/khtml/src/rendering/table_layout.h

SOURCES += \
    $$PWD/khtml/src/rendering/bidi.cpp \
    $$PWD/khtml/src/rendering/break_lines.cpp \
    $$PWD/khtml/src/rendering/counter_tree.cpp \
    $$PWD/khtml/src/rendering/enumerate.cpp \
    $$PWD/khtml/src/rendering/font.cpp \
    $$PWD/khtml/src/rendering/loading_icon.cpp \
    $$PWD/khtml/src/rendering/media_controls.cpp \
    $$PWD/khtml/src/rendering/render_arena.cpp \
    $$PWD/khtml/src/rendering/render_block.cpp \
    $$PWD/khtml/src/rendering/render_body.cpp \
    $$PWD/khtml/src/rendering/render_box.cpp \
    $$PWD/khtml/src/rendering/render_br.cpp \
    $$PWD/khtml/src/rendering/render_canvas.cpp \
    $$PWD/khtml/src/rendering/render_canvasimage.cpp \
    $$PWD/khtml/src/rendering/render_container.cpp \
    $$PWD/khtml/src/rendering/render_flow.cpp \
    $$PWD/khtml/src/rendering/render_form.cpp \
    $$PWD/khtml/src/rendering/render_frames.cpp \
    $$PWD/khtml/src/rendering/render_generated.cpp \
    $$PWD/khtml/src/rendering/render_image.cpp \
    $$PWD/khtml/src/rendering/render_inline.cpp \
    $$PWD/khtml/src/rendering/render_layer.cpp \
    $$PWD/khtml/src/rendering/render_line.cpp \
    $$PWD/khtml/src/rendering/render_list.cpp \
    $$PWD/khtml/src/rendering/render_media.cpp \
    $$PWD/khtml/src/rendering/render_object.cpp \
    $$PWD/khtml/src/rendering/render_position.cpp \
    $$PWD/khtml/src/rendering/render_replaced.cpp \
    $$PWD/khtml/src/rendering/render_style.cpp \
    $$PWD/khtml/src/rendering/render_table.cpp \
    $$PWD/khtml/src/rendering/render_text.cpp \
    $$PWD/khtml/src/rendering/table_layout.cpp

#xml
HEADERS += \
    $$PWD/khtml/src/xml/ClassNames.h \
    $$PWD/khtml/src/xml/dom_docimpl.h \
    $$PWD/khtml/src/xml/dom_elementimpl.h \
    $$PWD/khtml/src/xml/dom_nodeimpl.h \
    $$PWD/khtml/src/xml/dom_nodelistimpl.h \
    $$PWD/khtml/src/xml/dom_position.h \
    $$PWD/khtml/src/xml/dom_positioniterator.h \
    $$PWD/khtml/src/xml/dom_restyler.h \
    $$PWD/khtml/src/xml/dom_selection.h \
    $$PWD/khtml/src/xml/dom_stringimpl.h \
    $$PWD/khtml/src/xml/dom_textimpl.h \
    $$PWD/khtml/src/xml/dom_xmlimpl.h \
    $$PWD/khtml/src/xml/dom2_eventsimpl.h \
    $$PWD/khtml/src/xml/dom2_rangeimpl.h \
    $$PWD/khtml/src/xml/dom2_traversalimpl.h \
    $$PWD/khtml/src/xml/dom2_viewsimpl.h \
    $$PWD/khtml/src/xml/dom3_xpathimpl.h \
    $$PWD/khtml/src/xml/security_origin.h \
    $$PWD/khtml/src/xml/wa_selectors.h \
    $$PWD/khtml/src/xml/xml_tokenizer.h

SOURCES += \
    $$PWD/khtml/src/xml/ClassNames.cpp \
    $$PWD/khtml/src/xml/dom_docimpl.cpp \
    $$PWD/khtml/src/xml/dom_elementimpl.cpp \
    $$PWD/khtml/src/xml/dom_nodeimpl.cpp \
    $$PWD/khtml/src/xml/dom_nodelistimpl.cpp \
    $$PWD/khtml/src/xml/dom_position.cpp \
    $$PWD/khtml/src/xml/dom_positioniterator.cpp \
    $$PWD/khtml/src/xml/dom_restyler.cpp \
    $$PWD/khtml/src/xml/dom_selection.cpp \
    $$PWD/khtml/src/xml/dom_stringimpl.cpp \
    $$PWD/khtml/src/xml/dom_textimpl.cpp \
    $$PWD/khtml/src/xml/dom_xmlimpl.cpp \
    $$PWD/khtml/src/xml/dom2_eventsimpl.cpp \
    $$PWD/khtml/src/xml/dom2_rangeimpl.cpp \
    $$PWD/khtml/src/xml/dom2_viewsimpl.cpp \
    $$PWD/khtml/src/xml/security_origin.cpp \
    $$PWD/khtml/src/xml/wa_selectors.cpp \
    $$PWD/khtml/src/xml/xml_tokenizer.cpp

ENABLE_DOM2 {
SOURCES += \
    $$PWD/khtml/src/dom/dom2_traversal.cpp \
    $$PWD/khtml/src/xml/dom2_traversalimpl.cpp
}

#xpath: dom3
ENABLE_DOM3 {
HEADERS += \
    $$PWD/khtml/src/xpath/expression.h \
    $$PWD/khtml/src/xpath/functions.h \
    $$PWD/khtml/src/xpath/parsedstatement.h \
    $$PWD/khtml/src/xpath/parser.h \
    $$PWD/khtml/src/xpath/path.h \
    $$PWD/khtml/src/xpath/predicate.h \
    $$PWD/khtml/src/xpath/step.h \
    $$PWD/khtml/src/xpath/tokenizer.h \
    $$PWD/khtml/src/xpath/util.h \
    $$PWD/khtml/src/xpath/variablereference.h

SOURCES += \
    $$PWD/khtml/src/dom/dom3_xpath.cpp \
    $$PWD/khtml/src/xml/dom3_xpathimpl.cpp \
    $$PWD/khtml/src/xpath/expression.cpp \
    $$PWD/khtml/src/xpath/functions.cpp \
    $$PWD/khtml/src/xpath/parsedstatement.cpp \
    $$PWD/khtml/src/xpath/parser.cpp \
    $$PWD/khtml/src/xpath/path.cpp \
    $$PWD/khtml/src/xpath/predicate.cpp \
    $$PWD/khtml/src/xpath/step.cpp \
    $$PWD/khtml/src/xpath/tokenizer.cpp \
    $$PWD/khtml/src/xpath/util.cpp \
    $$PWD/khtml/src/xpath/variablereference.cpp
}

RESOURCES += \
    $$PWD/khtml/khtml_resource.qrc
