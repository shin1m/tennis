#ifndef XML_READER_H
#define XML_READER_H

#include <codecvt>
#include <locale>
#include <stdexcept>
#include <string>
#include <vector>
#include <libxml/xmlreader.h>

class t_text_reader
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> v_convert;
	xmlTextReaderPtr v_reader;

protected:
	std::string f_convert(const std::wstring& a_string)
	{
		return v_convert.to_bytes(a_string);
	}
	std::wstring f_convert(const std::string& a_string)
	{
		return v_convert.from_bytes(a_string);
	}
	const xmlChar* f_cast(const std::string& a_string)
	{
		return reinterpret_cast<const xmlChar*>(a_string.c_str());
	}
	void f_void(int a_code)
	{
		if (a_code != 0) throw std::runtime_error("error occurred.");
	}
	bool f_boolean(int a_code)
	{
		if (a_code == -1) throw std::runtime_error("error occurred.");
		return a_code != 0;
	}
	int f_integer(int a_code)
	{
		if (a_code == -1) throw std::runtime_error("error occurred.");
		return a_code;
	}
	std::wstring f_string(const xmlChar* a_p)
	{
		return a_p == NULL ? std::wstring() : f_convert(reinterpret_cast<const char*>(a_p));
	}
	std::wstring f_string(xmlChar* a_p)
	{
		if (a_p == NULL) return std::wstring();
		std::wstring s = f_convert(reinterpret_cast<char*>(a_p));
		xmlFree(a_p);
		return s;
	}

public:
	t_text_reader(const std::wstring& a_uri) : v_reader(xmlNewTextReaderFilename(f_convert(a_uri).c_str()))
	{
	}
	~t_text_reader()
	{
		xmlFreeTextReader(v_reader);
	}
	bool f_read()
	{
		return f_boolean(xmlTextReaderRead(v_reader));
	}
	std::wstring f_read_string()
	{
		return f_string(xmlTextReaderReadString(v_reader));
	}
	bool f_read_attribute_value()
	{
		return f_boolean(xmlTextReaderReadAttributeValue(v_reader));
	}
	int f_attribute_count()
	{
		return f_integer(xmlTextReaderAttributeCount(v_reader));
	}
	int f_depth()
	{
		return f_integer(xmlTextReaderDepth(v_reader));
	}
	bool f_has_attributes()
	{
		return f_boolean(xmlTextReaderHasAttributes(v_reader));
	}
	bool f_has_value()
	{
		return f_boolean(xmlTextReaderHasValue(v_reader));
	}
	bool f_is_default()
	{
		return f_boolean(xmlTextReaderIsDefault(v_reader));
	}
	bool f_is_empty_element()
	{
		return f_boolean(xmlTextReaderIsEmptyElement(v_reader));
	}
	int f_node_type()
	{
		return f_integer(xmlTextReaderNodeType(v_reader));
	}
	int f_quote_character()
	{
		return f_integer(xmlTextReaderQuoteChar(v_reader));
	}
	int f_read_state()
	{
		return f_integer(xmlTextReaderReadState(v_reader));
	}
	bool f_is_namespace_decl()
	{
		return f_boolean(xmlTextReaderIsNamespaceDecl(v_reader));
	}
	std::wstring f_base_uri()
	{
		return f_string(xmlTextReaderConstBaseUri(v_reader));
	}
	std::wstring f_local_name()
	{
		return f_string(xmlTextReaderConstLocalName(v_reader));
	}
	std::wstring f_name()
	{
		return f_string(xmlTextReaderConstName(v_reader));
	}
	std::wstring f_namespace_uri()
	{
		return f_string(xmlTextReaderConstNamespaceUri(v_reader));
	}
	std::wstring f_prefix()
	{
		return f_string(xmlTextReaderConstPrefix(v_reader));
	}
	std::wstring f_xml_lang()
	{
		return f_string(xmlTextReaderConstXmlLang(v_reader));
	}
	const xmlChar* f_const_string(const xmlChar* a_string)
	{
		return xmlTextReaderConstString(v_reader, a_string);
	}
	std::wstring f_value()
	{
		return f_string(xmlTextReaderConstValue(v_reader));
	}
	void f_close()
	{
		f_void(xmlTextReaderClose(v_reader));
	}
	std::wstring f_get_attribute_no(int a_no)
	{
		return f_string(xmlTextReaderGetAttributeNo(v_reader, a_no));
	}
	std::wstring f_get_attribute(const std::wstring& a_name)
	{
		return f_string(xmlTextReaderGetAttribute(v_reader, f_cast(f_convert(a_name))));
	}
	std::wstring f_get_attribute_ns(const std::wstring& a_name, const std::wstring& a_uri)
	{
		return f_string(xmlTextReaderGetAttributeNs(v_reader, f_cast(f_convert(a_name)), f_cast(f_convert(a_uri))));
	}
	std::wstring f_lookup_namespace(const std::wstring& a_prefix)
	{
		return f_string(xmlTextReaderLookupNamespace(v_reader, f_cast(f_convert(a_prefix))));
	}
	bool f_move_to_attribute_no(int a_no)
	{
		return f_boolean(xmlTextReaderMoveToAttributeNo(v_reader, a_no));
	}
	bool f_move_to_attribute(const std::wstring& a_name)
	{
		return f_boolean(xmlTextReaderMoveToAttribute(v_reader, f_cast(f_convert(a_name))));
	}
	bool f_move_to_attribute_ns(const std::wstring& a_name, const std::wstring& a_uri)
	{
		return f_boolean(xmlTextReaderMoveToAttributeNs(v_reader, f_cast(f_convert(a_name)), f_cast(f_convert(a_uri))));
	}
	bool f_move_to_first_attribute()
	{
		return f_boolean(xmlTextReaderMoveToFirstAttribute(v_reader));
	}
	bool f_move_to_next_attribute()
	{
		return f_boolean(xmlTextReaderMoveToNextAttribute(v_reader));
	}
	bool f_move_to_element()
	{
		return f_boolean(xmlTextReaderMoveToElement(v_reader));
	}
	bool f_normalization()
	{
		return f_boolean(xmlTextReaderNormalization(v_reader));
	}
	std::wstring f_encoding()
	{
		return f_string(xmlTextReaderConstEncoding(v_reader));
	}
	void f_set_parser_property(int a_property, int a_value)
	{
		f_void(xmlTextReaderSetParserProp(v_reader, a_property, a_value));
	}
	int f_get_parser_property(int a_property)
	{
		return f_integer(xmlTextReaderGetParserProp(v_reader, a_property));
	}
	int f_get_parser_line_number()
	{
		return xmlTextReaderGetParserLineNumber(v_reader);
	}
	int f_get_parser_column_number()
	{
		return xmlTextReaderGetParserColumnNumber(v_reader);
	}
	bool f_next()
	{
		return f_boolean(xmlTextReaderNext(v_reader));
	}
	bool f_next_sibling()
	{
		return f_boolean(xmlTextReaderNextSibling(v_reader));
	}
	bool f_is_valid()
	{
		return f_boolean(xmlTextReaderIsValid(v_reader));
	}
	std::wstring f_xml_version()
	{
		return f_string(xmlTextReaderConstXmlVersion(v_reader));
	}
	bool f_standalone()
	{
		return f_boolean(xmlTextReaderStandalone(v_reader));
	}
	int f_byte_consumed()
	{
		return xmlTextReaderByteConsumed(v_reader);
	}
	void f_set_error_handler(xmlTextReaderErrorFunc a_function, void* a_argument)
	{
		xmlTextReaderSetErrorHandler(v_reader, a_function, a_argument);
	}
	void f_set_structured_error_handler(xmlStructuredErrorFunc a_function, void* a_argument)
	{
		xmlTextReaderSetStructuredErrorHandler(v_reader, a_function, a_argument);
	}
	void f_get_error_handler(xmlTextReaderErrorFunc* a_function, void** a_argument)
	{
		xmlTextReaderGetErrorHandler(v_reader, a_function, a_argument);
	}
};

class t_reader : public t_text_reader
{
	int v_type = XML_READER_TYPE_NONE;

public:
	using t_text_reader::t_text_reader;
	void f_read_next()
	{
		v_type = f_read() ? f_node_type() : XML_READER_TYPE_NONE;
	}
	int f_type() const
	{
		return v_type;
	}
	void f_move_to_tag()
	{
		while (v_type != XML_READER_TYPE_NONE && v_type != XML_READER_TYPE_ELEMENT && v_type != XML_READER_TYPE_END_ELEMENT) f_read_next();
	}
	bool f_is_start_element(const wchar_t* a_name)
	{
		f_move_to_tag();
		return v_type == XML_READER_TYPE_ELEMENT && f_local_name() == a_name;
	}
	void f_check_start_element(const wchar_t* a_name)
	{
		if (!f_is_start_element(a_name)) throw std::runtime_error(f_convert(std::wstring(L"must be element: ") + a_name));
	}
	bool f_start_element(const wchar_t* a_name)
	{
		f_check_start_element(a_name);
		bool b = f_is_empty_element();
		f_read_next();
		return !b;
	}
	void f_end_element()
	{
		f_move_to_tag();
		if (v_type != XML_READER_TYPE_END_ELEMENT) throw std::runtime_error("must be end of element.");
		f_read_next();
	}
	std::wstring f_read_element_text()
	{
		if (f_is_empty_element()) {
			f_read_next();
			return std::wstring();
		}
		std::wstring text;
		f_read_next();
		while (v_type != XML_READER_TYPE_END_ELEMENT) {
			if (v_type == XML_READER_TYPE_TEXT || v_type == XML_READER_TYPE_CDATA) {
				text += f_value();
				f_read_next();
			} else if (v_type == XML_READER_TYPE_ELEMENT) {
				text += f_read_element_text();
			} else {
				f_read_next();
			}
		}
		f_read_next();
		return text;
	}
	std::wstring f_read_element(const wchar_t* a_name)
	{
		f_check_start_element(a_name);
		return f_read_element_text();
	}
	template<typename T_elements, typename... T>
	void f_parse_elements(T_elements a_elements, T&&... a_x)
	{
		f_read_next();
		while (true) {
			f_move_to_tag();
			if (v_type != XML_READER_TYPE_ELEMENT) break;
			auto i = a_elements.find(f_local_name());
			if (i == a_elements.end()) {
				f_read_element_text();
			} else {
				i->second(std::forward<T>(a_x)...);
			}
		}
		f_read_next();
	};
};

#endif
