#include "loadLuaFromZip.hpp"
#include <zip.h>
#include <vector>
#include <iostream>

/**
 * @brief Loads Lua scripts from a ZIP archive into the given Lua state.
 *
 * This function opens a ZIP archive, iterates through its entries, and loads any Lua scripts
 * found within into the provided Lua state.
 *
 * @param L The Lua state to load the scripts into.
 * @param zipname The path to the ZIP archive to be opened.
 */
void loadLuaFromZip(lua_State *L, const std::string &zipname) {
    int err = 0;
    zip *archive = zip_open(zipname.c_str(), ZIP_RDONLY, &err);
    if (archive == nullptr) {
        std::cerr << "Failed to open ZIP archive " << zipname << ": " << std::system_error(err, std::generic_category()).what() << std::endl;
        return;
    }

    for (int i = 0; i < zip_get_num_entries(archive, 0); i++) {
        struct zip_stat st;
        zip_stat_index(archive, i, 0, &st);

        std::string fileName(st.name);
        if (fileName != "main.lua" && fileName.find(".lua") == std::string::npos) {
            continue;
        }

        zip_file *file = zip_fopen_index(archive, i, 0);
        if (!file) {
            std::cerr << "Failed to open file " << st.name << " in ZIP archive." << std::endl;
            continue;
        }

        std::vector<char> buffer(st.size);
        zip_fread(file, buffer.data(), st.size);
        zip_fclose(file);

        if (luaL_loadbuffer(L, buffer.data(), st.size, st.name) || lua_pcall(L, 0, 0, 0)) {
            std::cerr << "Failed to load script " << st.name << ": " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1); // Remove error message from stack
        }
    }

    zip_close(archive);
}

