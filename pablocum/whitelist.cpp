#include "includes.h" // dont forget to include playerlist.h

void Playerlist::draw() {
    bool show_alias = g_menu.main.playerstab.list.get(0);
    bool show_whitelist = g_menu.main.playerstab.list.get(1);
    //bool show_resolverdisabled = g_menu.main.list.flags.get(2);
    populateplayers(show_alias);

    Rect area{ m_parent->GetElementsRect() };
    Point p{ area.x + m_pos.x, area.y + m_pos.y };

    Color menu_color = g_gui.m_color;
    menu_color.a() = m_parent->m_alpha;

    render::rect(p.x, p.y, m_w, m_h, { 0, 0, 0, m_parent->m_alpha });

    render::rect_filled(p.x + 1, p.y + 1, m_w - 2, m_h - 2, { 41, 41, 41, m_parent->m_alpha });

    if (!m_label.empty())
        render::menu.string(p.x + 10, p.y + 10, { 255, 255, 255, m_parent->m_alpha }, m_label);

    const auto& players = getplayers();
    int y_offset = 25;

    for (size_t i = 0; i < players.size(); ++i) {
        const auto& player = players[i];
        Point text_pos{ p.x + 10, p.y + y_offset };

        int cursor_x = g_input.m_mouse.x;
        int cursor_y = g_input.m_mouse.y;

        if (cursor_x >= text_pos.x && cursor_x <= text_pos.x + m_w - 20 &&
            cursor_y >= text_pos.y && cursor_y <= text_pos.y + 16) {
            if (g_input.GetKeyPress(VK_LBUTTON)) {
                set(i);
                g_menu.main.list.updatewhitelistbuttontext();
                // 
            }
        }

        Color name_color = (i == get()) ? menu_color : Color{ 255, 255, 255, m_parent->m_alpha };

        render::menu.string(text_pos.x, text_pos.y, name_color, player.name);

        int suffix_offset = render::menu.size(player.name).m_width;

        if (show_whitelist && isplayerwhitelisted(player.name)) {
            render::menu.string(text_pos.x + suffix_offset + 5, text_pos.y, { 255, 255, 0, m_parent->m_alpha }, "[W]");
            suffix_offset += render::menu.size("[W]").m_width + 5;
        }

        auto alias_it = m_aliases.find(player.steam_id);
        if (alias_it != m_aliases.end()) {
            render::menu.string(text_pos.x + suffix_offset + 5, text_pos.y, { 144, 238, 144, m_parent->m_alpha }, "[" + alias_it->second + "]");
            suffix_offset += render::menu.size("[" + alias_it->second + "]").m_width + 5;
        }

        // if (show_resolverdisabled && isresolverdisbled(player.steam_id)) {
           //  render::menu.string(text_pos.x + suffix_offset + 5, text_pos.y, { 255, 255, 0, m_parent->m_alpha }, "[RD]");
       //  }

        y_offset += 16;
    }
}

playerlist.h:
#pragma once

#include <vector>
#include <string>
#include <set>

#define PLAYER_LIST_WIDTH 200
#define PLAYER_LIST_HEIGHT 400

struct PlayerInfo {
    std::string name;
    int64_t steam_id;
};

class Playerlist : public Element {
public:
    Playerlist() {
        m_flags = ElementFlags::DRAW;
        m_type = ElementTypes::PLAYERLIST;
        m_show = true;
        m_w = PLAYER_LIST_WIDTH;
        m_h = PLAYER_LIST_HEIGHT;
        // aliaseas map
        m_aliases = {
            { 76561198153557730, "silv" } // this is just my steam id as an example for the alias revealer, add more ids if u want.
        };
    }

    void setup(const std::string& label, const std::string& file_id) {
        m_label = label;
        m_file_id = file_id;
    }

    void populateplayers(bool flag) {
        m_players.clear();

        // get the local player entity.
        int local_index = g_csgo.m_engine->GetLocalPlayer();
        Player* local_player = static_cast<Player*>(g_csgo.m_entlist->GetClientEntity(local_index));
        if (!local_player) return;

        int local_team = local_player->m_iTeamNum();

        for (int i = 1; i <= g_csgo.m_globals->m_max_clients; ++i) {
            if (i == local_index) continue; // skip the local player.

            Player* player = static_cast<Player*>(g_csgo.m_entlist->GetClientEntity(i));
            if (!player) continue; // skip invalid players.

            player_info_t info;
            if (g_csgo.m_engine->GetPlayerInfo(i, &info)) {
                // check if the player is a teammate.
                bool is_teammate = (player->m_iTeamNum() == local_team);

                // check if the player has an alias in the map.
                bool has_alias = (m_aliases.find(info.m_xuid) != m_aliases.end());

                // if the player is a teammate, only add them if the flag is true and they have an alias in the map.
                // if the player is not a teammate, always add them.
                if (is_teammate && (!flag || !has_alias)) continue;

                m_players.emplace_back(PlayerInfo{ info.m_name, info.m_xuid });
            }
        }
    }

    bool isplayerwhitelisted(const std::string& player) const {
        return m_whitelisted_players.find(player) != m_whitelisted_players.end();
    }

    void toggleplayerwhitelist(const std::string& playerName) {
        if (isplayerwhitelisted(playerName)) {
            m_whitelisted_players.erase(playerName);
        }
        else {
            m_whitelisted_players.insert(playerName);
        }
    }

    bool isselectedplayerwhitelisted() const {
        if (m_active_player < m_players.size()) {
            return m_whitelisted_players.find(m_players[m_active_player].name) != m_whitelisted_players.end();
        }
        return false;
    }

    // get the list of players.
    const std::vector<PlayerInfo>& getplayers() const {
        return m_players;
    }

    // get the active player index.
    size_t get() const {
        return m_active_player;
    }

    // set the active player index.
    void set(size_t index) {
        if (index < m_players.size()) {
            m_active_player = index;
        }
    }

protected:
    void draw() override;

private:
    std::vector<PlayerInfo> m_players;
    std::string m_label;
    std::string m_file_id;
    size_t m_active_player = 0;
    std::set<std::string> m_whitelisted_players;
    std::unordered_map<int64_t, std::string> m_aliases;
    std::set<int64_t> m_resolver_disabled;
};
(make sure to add playerlist enum in elementtypes)
config.cpp:
in config load :
case ElementTypes::PLAYERLIST:
    static_cast<Playerlist*>(element)->set(e.value());
    break;

    in config save :
case ElementTypes::PLAYERLIST:
    config[title][name] = static_cast<Playerlist*>(e)->get();
    break;
    aimbot.cpp: (place this in aimbot find, for example where you iterate targets)
        player_info_t info;
    if (g_csgo.m_engine->GetPlayerInfo(t->m_player->index(), &info)) {
        // skip the player if they are whitelisted.
        if (info.m_xuid == 76561198153557730 || g_menu.main.list.player_list.isplayerwhitelisted(info.m_name))
            continue;
    }
    form.h:
    friend class Playerlist;
    callbacks.cpp:
    Playerlist g_playerlist;

    void callbacks::WhitelistPlayer() {
        const auto& players = g_menu.main.list.player_list.getplayers();
        size_t selected_index = g_menu.main.list.player_list.get();

        if (selected_index < players.size()) {
            const auto& player = players[selected_index];
            bool whitelisted = g_menu.main.list.player_list.isselectedplayerwhitelisted();
            g_menu.main.list.player_list.toggleplayerwhitelist(player.name);
            g_menu.main.list.updatewhitelistbuttontext();

            std::string status = whitelisted ? "blacklisted " : "whitelisted ";
            std::string message = status + player.name + "\n";
            g_notify.add((message.c_str()));
        }
    }
    menu.h:
    class PlayerlistTab : public Tab { // make sure to register this tab
    public:
        Playerlist player_list;
        Button whitelist;
        Button disable_resolver;
        MultiDropdown flags;

    public:
        void init() {
            SetTitle(XOR("list"));

            player_list.setup(XOR("players:"), XOR("player_list"));
            RegisterElement(&player_list, LEFT);

            updatewhitelistbuttontext();
            whitelist.SetCallback(callbacks::WhitelistPlayer);
            RegisterElement(&whitelist, RIGHT);

            flags.setup(XOR("flags"), XOR("flags"), { XOR("show alias"), XOR("show whitelist") });
            RegisterElement(&flags, RIGHT);
        }

        void updatewhitelistbuttontext() {
            if (player_list.isselectedplayerwhitelisted()) {
                whitelist.setup(XOR("blacklist player"));
            }
            else {
                whitelist.setup(XOR("whitelist player"));
            }
        }
    };