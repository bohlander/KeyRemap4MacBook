#include <exception>
#include <iostream>
#include <sstream>
#include "pqrs/string.hpp"
#include "pqrs/xml_compiler.hpp"

namespace pqrs {
  static void
  append_environments_to_replacement(pqrs::string::replacement& r) {
    if (r.find("ENV_HOME") == r.end()) {
      const char* p = std::getenv("HOME");
      if (! p) {
        p = "";
      }
      r["ENV_HOME"] = p;
    }
  }

  void
  xml_compiler::reload(void)
  {
    error_information_.clear();
    replacement_warnings_.clear();
    replacement_.clear();
    symbol_map_.clear();
    modifier_map_.clear();
    app_vector_.clear();
    window_name_vector_.clear();
    vk_change_inputsource_map_.clear();
    inputsource_vector_.clear();
    vk_open_url_map_.clear();
    identifier_map_.clear();
    essential_configurations_.clear();
    remapclasses_initialize_vector_.clear();
    preferences_checkbox_node_tree_.clear();
    preferences_number_node_tree_.clear();

    try {
      std::string private_xml_file_path = make_file_path(private_xml_directory_, "private.xml");

      // ------------------------------------------------------------
      // Loading replacement
      {
        replacement_loader loader(*this, replacement_);

        pqrs::string::replacement dummy; // Use dummy replacement when we read <replacementdef>.
        append_environments_to_replacement(dummy);

        // private.xml
        {
          ptree_ptr ptree_ptr;
          read_xml_(ptree_ptr, private_xml_file_path, dummy);
          if (ptree_ptr) {
            loader.traverse(make_extracted_ptree(*ptree_ptr, private_xml_file_path, dummy));
          }
        }
        // replacementdef.xml
        {
          ptree_ptr ptree_ptr;
          std::string xml_file_path = make_file_path(system_xml_directory_,  "replacementdef.xml");
          read_xml_(ptree_ptr, xml_file_path, dummy);
          if (ptree_ptr) {
            loader.traverse(make_extracted_ptree(*ptree_ptr, xml_file_path, dummy));
          }
        }

        append_environments_to_replacement(replacement_);
      }

      // Reset replacement warnings that occur during loading replacement.
      replacement_warnings_.clear();

      // ------------------------------------------------------------
      // Then, we read private.xml with replacement and loaders share it.

      ptree_ptr private_xml_ptree_ptr;
      read_xml_(private_xml_ptree_ptr, private_xml_file_path);
      if (private_xml_ptree_ptr && private_xml_ptree_ptr->empty()) {
        private_xml_ptree_ptr.reset();
      }

      // symbol_map
      {
        symbol_map_loader loader(*this, symbol_map_);

        if (private_xml_ptree_ptr) {
          loader.traverse(make_extracted_ptree(*private_xml_ptree_ptr, private_xml_file_path));
        }

        loader_wrapper<symbol_map_loader>::traverse_system_xml(*this, loader, "symbol_map.xml");
      }

      // modifier
      {
        modifier_loader loader(*this,
                               symbol_map_,
                               remapclasses_initialize_vector_,
                               identifier_map_,
                               modifier_map_);

        if (private_xml_ptree_ptr) {
          loader.traverse(make_extracted_ptree(*private_xml_ptree_ptr, private_xml_file_path));
        }

        loader_wrapper<modifier_loader>::traverse_system_xml(*this, loader, "modifierdef.xml");
      }

      // app
      {
        app_loader loader(*this, symbol_map_, app_vector_);

        if (private_xml_ptree_ptr) {
          loader.traverse(make_extracted_ptree(*private_xml_ptree_ptr, private_xml_file_path));
        }

        loader_wrapper<app_loader>::traverse_system_xml(*this, loader, "appdef.xml");
      }

      // window_name
      {
        window_name_loader loader(*this, symbol_map_, window_name_vector_);

        if (private_xml_ptree_ptr) {
          loader.traverse(make_extracted_ptree(*private_xml_ptree_ptr, private_xml_file_path));
        }

        loader_wrapper<window_name_loader>::traverse_system_xml(*this, loader, "windownamedef.xml");
      }

      // device
      {
        device_loader loader(*this, symbol_map_);

        if (private_xml_ptree_ptr) {
          loader.traverse(make_extracted_ptree(*private_xml_ptree_ptr, private_xml_file_path));
        }

        loader_wrapper<device_loader>::traverse_system_xml(*this, loader, "devicevendordef.xml");
        loader_wrapper<device_loader>::traverse_system_xml(*this, loader, "deviceproductdef.xml");
        loader_wrapper<device_loader>::traverse_system_xml(*this, loader, "devicelocationdef.xml");
      }

      // inputsource
      {
        inputsource_loader loader(*this,
                                  symbol_map_,
                                  remapclasses_initialize_vector_,
                                  identifier_map_,
                                  vk_change_inputsource_map_,
                                  inputsource_vector_);

        if (private_xml_ptree_ptr) {
          loader.traverse(make_extracted_ptree(*private_xml_ptree_ptr, private_xml_file_path));
        }

        loader_wrapper<inputsource_loader>::traverse_system_xml(*this, loader, "vkchangeinputsourcedef.xml");
        loader_wrapper<inputsource_loader>::traverse_system_xml(*this, loader, "inputsourcedef.xml");
      }

      // url
      {
        url_loader loader(*this,
                          symbol_map_,
                          remapclasses_initialize_vector_,
                          identifier_map_,
                          vk_open_url_map_);

        if (private_xml_ptree_ptr) {
          loader.traverse(make_extracted_ptree(*private_xml_ptree_ptr, private_xml_file_path));
        }

        loader_wrapper<url_loader>::traverse_system_xml(*this, loader, "vkopenurldef.xml");
      }

      // config_index, remapclasses_initialize_vector, preferences_node
      {
        std::string checkbox_xml_file_path = make_file_path(system_xml_directory_, "checkbox.xml");
        ptree_ptr checkbox_xml_ptree_ptr;
        read_xml_(checkbox_xml_ptree_ptr, checkbox_xml_file_path);

        std::string number_xml_file_path = make_file_path(system_xml_directory_, "number.xml");
        ptree_ptr number_xml_ptree_ptr;
        read_xml_(number_xml_ptree_ptr, number_xml_file_path);

        // remapclasses_initialize_vector
        {
          // prepare
          {
            remapclasses_initialize_vector_prepare_loader<preferences_node_tree<preferences_number_node> > loader(*this, symbol_map_, essential_configurations_, &preferences_number_node_tree_);

            if (number_xml_ptree_ptr) {
              loader.traverse(make_extracted_ptree(*number_xml_ptree_ptr, number_xml_file_path));
              loader.fixup();
            }
            loader.cleanup();
          }
          {
            remapclasses_initialize_vector_prepare_loader<preferences_node_tree<preferences_checkbox_node> > loader(*this, symbol_map_, essential_configurations_, &preferences_checkbox_node_tree_);

            if (private_xml_ptree_ptr) {
              loader.traverse(make_extracted_ptree(*private_xml_ptree_ptr, private_xml_file_path));
              loader.fixup();
            }
            if (checkbox_xml_ptree_ptr) {
              loader.traverse(make_extracted_ptree(*checkbox_xml_ptree_ptr, checkbox_xml_file_path));
              loader.fixup();
            }
            loader.cleanup();
          }

          // ----------------------------------------
          {
            remapclasses_initialize_vector_loader loader(*this,
                                                         symbol_map_,
                                                         remapclasses_initialize_vector_,
                                                         identifier_map_);
            if (private_xml_ptree_ptr) {
              loader.traverse(make_extracted_ptree(*private_xml_ptree_ptr, private_xml_file_path), "");
            }
            if (checkbox_xml_ptree_ptr) {
              loader.traverse(make_extracted_ptree(*checkbox_xml_ptree_ptr, checkbox_xml_file_path), "");
            }
          }

          remapclasses_initialize_vector_.freeze();
        }
      }

      if (! replacement_warnings_.empty()) {
        error_information_.set(replacement_warnings_);
      }

    } catch (std::exception& e) {
      error_information_.set(e.what());
    }
  }

  void
  xml_compiler::read_xml_(ptree_ptr& out,
                          const std::string& file_path,
                          const pqrs::string::replacement& replacement) const
  {
    try {
      out.reset(new boost::property_tree::ptree());

      std::string xml;
      if (replacement.empty()) {
        pqrs::string::string_from_file(xml, file_path.c_str());
      } else {
        pqrs::string::string_by_replacing_double_curly_braces_from_file(xml,
                                                                        replacement_warnings_,
                                                                        file_path.c_str(),
                                                                        replacement);
      }
      if (xml.empty()) {
        error_information_.set(file_path + " is not found.");
        return;
      }

      std::stringstream istream(xml, std::stringstream::in);

      int flags = boost::property_tree::xml_parser::no_comments;

      boost::property_tree::read_xml(istream, *out, flags);

    } catch (std::exception& e) {
      std::string what = e.what();

      // Hack:
      // boost::property_tree::read_xml throw exception with filename.
      // But, when we call read_xml with stream, the filename becomes "unspecified file" as follow.
      //
      // <unspecified file>(4): expected element name
      //
      // So, we change "unspecified file" to file name by ourself.
      boost::replace_first(what, "<unspecified file>", std::string("<") + file_path + ">");

      error_information_.set(what);
    }
  }

  boost::optional<const std::string&>
  xml_compiler::get_identifier(int config_index) const
  {
    auto it = identifier_map_.find(config_index);
    if (it == identifier_map_.end()) {
      return boost::none;
    }
    return it->second;
  }

  boost::optional<int>
  xml_compiler::get_config_index(const std::string& identifier) const
  {
    for (const auto& it : identifier_map_) {
      if (it.second == identifier) {
        return it.first;
      }
    }
    return boost::none;
  }

  uint32_t
  xml_compiler::get_appid(const std::string& application_identifier) const
  {
    for (const auto& it : app_vector_) {
      if (! it) continue;

      if (it->is_rules_matched(application_identifier)) {
        auto name = it->get_name();
        if (! name) goto notfound;

        auto v = symbol_map_.get_optional(std::string("ApplicationType::") + *name);
        if (! v) goto notfound;

        return *v;
      }
    }

  notfound:
    // return ApplicationType::UNKNOWN (== 0)
    return 0;
  }

  uint32_t
  xml_compiler::get_windownameid(const std::string& window_name) const
  {
    for (const auto& it : window_name_vector_) {
      if (! it) continue;

      if (it->is_rules_matched(window_name)) {
        auto name = it->get_name();
        if (! name) goto notfound;

        auto v = symbol_map_.get_optional(std::string("WindowName::") + *name);
        if (! v) goto notfound;

        return *v;
      }
    }

  notfound:
    // return WindowName::UNKNOWN (== 0)
    return 0;
  }

  bool
  xml_compiler::is_vk_change_inputsource_matched(uint32_t keycode,
                                                 const std::string& languagecode,
                                                 const std::string& inputsourceid,
                                                 const std::string& inputmodeid) const
  {
    auto it = vk_change_inputsource_map_.find(keycode);
    if (it == vk_change_inputsource_map_.end()) {
      return false;
    }

    if (! it->second) {
      return false;
    }
    return it->second->is_rules_matched(languagecode, inputsourceid, inputmodeid);
  }

  void
  xml_compiler::get_inputsourceid(uint32_t& inputsource,
                                  uint32_t& inputsource_detail,
                                  const std::string& languagecode,
                                  const std::string& inputsourceid,
                                  const std::string& inputmodeid) const
  {
    inputsource        = 0; // InputSource::NONE
    inputsource_detail = 0; // InputSourceDetail::NONE

    for (const auto& it : inputsource_vector_) {
      if (! it) continue;

      if (it->is_rules_matched(languagecode, inputsourceid, inputmodeid)) {
        {
          auto v = symbol_map_.get_optional(std::string("InputSource::") + *(it->get_name()));
          if (! v) return;
          inputsource = *v;
        }
        {
          if (it->get_detail()) {
            auto v = symbol_map_.get_optional(std::string("InputSourceDetail::") + *(it->get_detail()));
            if (! v) return;
            inputsource_detail = *v;
          } else {
            auto v = symbol_map_.get_optional(std::string("InputSourceDetail::") + *(it->get_name()));
            if (! v) return;
            inputsource_detail = *v;
          }
        }
        return;
      }
    }
  }

  boost::optional<const std::string&>
  xml_compiler::get_url(int keycode) const
  {
    auto it = vk_open_url_map_.find(keycode);
    if (it == vk_open_url_map_.end()) {
      return boost::none;
    }

    if (! it->second) {
      return boost::none;
    }
    return it->second->get_url();
  }

  boost::optional<const std::string&>
  xml_compiler::get_url_type(int keycode) const
  {
    auto it = vk_open_url_map_.find(keycode);
    if (it == vk_open_url_map_.end()) {
      return boost::none;
    }

    if (! it->second) {
      return boost::none;
    }
    return it->second->get_type();
  }

  bool
  xml_compiler::valid_identifier_(const std::string& identifier, const std::string& parent_tag_name) const
  {
    if (identifier.empty()) {
      error_information_.set("Empty <identifier>.");
      return false;
    }

    if (parent_tag_name != "item") {
      error_information_.set(boost::format("<identifier> must be placed directly under <item>:\n"
                                           "\n"
                                           "<identifier>%1%</identifier>") %
                             identifier);
      return false;
    }

    return true;
  }

  bool
  xml_compiler::debug_get_initialize_vector(std::vector<uint32_t>& out, const std::string& raw_identifier) const
  {
    std::string identifier = raw_identifier;
    normalize_identifier_(identifier);

    uint32_t target_config_index = symbol_map_.get("ConfigIndex", identifier);

    const auto& initialize_vector = remapclasses_initialize_vector_.get();

    size_t i = 1;
    for (;;) {
      if (i >= initialize_vector.size()) return false;

      uint32_t count = initialize_vector.at(i);
      ++i;
      if (count == 0) return false;

      uint32_t config_index = initialize_vector.at(i);
      if (target_config_index != config_index) {
        i += count;
        continue;
      }

      out.clear();
      ++i;
      --count;
      while (count > 0) {
        out.push_back(initialize_vector.at(i));
        ++i;
        --count;
      }
      return true;
    }

    return false;
  }
}
