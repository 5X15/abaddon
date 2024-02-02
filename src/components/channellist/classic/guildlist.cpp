#include "guildlist.hpp"

#include "abaddon.hpp"
#include "guildlistfolderitem.hpp"

class GuildListDMsButton : public Gtk::EventBox {
public:
    GuildListDMsButton() {
        set_size_request(48, 48);

        m_img.property_icon_name() = "user-available-symbolic"; // meh
        m_img.property_icon_size() = Gtk::ICON_SIZE_DND;
        add(m_img);
        show_all_children();
    }

private:
    Gtk::Image m_img;
};

GuildList::GuildList() {
    get_style_context()->add_class("guild-list");
    set_selection_mode(Gtk::SELECTION_NONE);
    show_all_children();
}

void GuildList::UpdateListing() {
    auto &discord = Abaddon::Get().GetDiscordClient();

    Clear();

    auto *dms = Gtk::make_managed<GuildListDMsButton>();
    dms->show();
    dms->signal_button_press_event().connect([this](GdkEventButton *ev) -> bool {
        if (ev->type == GDK_BUTTON_PRESS && ev->button == GDK_BUTTON_PRIMARY) {
            m_signal_dms_selected.emit();
        }
        return false;
    });
    add(*dms);

    // does this function still even work ??lol
    const auto folders = discord.GetUserSettings().GuildFolders;
    const auto guild_ids = discord.GetUserSortedGuilds();

    // same logic from ChannelListTree

    std::set<Snowflake> foldered_guilds;
    for (const auto &group : folders) {
        foldered_guilds.insert(group.GuildIDs.begin(), group.GuildIDs.end());
    }

    for (auto iter = guild_ids.crbegin(); iter != guild_ids.crend(); iter++) {
        if (foldered_guilds.find(*iter) == foldered_guilds.end()) {
            AddGuild(*iter);
        }
    }

    for (const auto &group : folders) {
        AddFolder(group);
    }
}

void GuildList::AddGuild(Snowflake id) {
    if (auto item = CreateGuildWidget(id)) {
        item->show();
        add(*item);
    }
}

GuildListGuildItem *GuildList::CreateGuildWidget(Snowflake id) {
    const auto guild = Abaddon::Get().GetDiscordClient().GetGuild(id);
    if (!guild.has_value()) return nullptr;

    auto *item = Gtk::make_managed<GuildListGuildItem>(*guild);
    item->signal_button_press_event().connect([this, id](GdkEventButton *event) -> bool {
        if (event->type == GDK_BUTTON_PRESS && event->button == GDK_BUTTON_PRIMARY) {
            m_signal_guild_selected.emit(id);
        }
        return true;
    });

    return item;
}

void GuildList::AddFolder(const UserSettingsGuildFoldersEntry &folder) {
    // groups with no ID arent actually folders
    if (!folder.ID.has_value()) {
        if (!folder.GuildIDs.empty()) {
            AddGuild(folder.GuildIDs[0]);
        }
        return;
    }

    auto *folder_widget = Gtk::make_managed<GuildListFolderItem>(folder);
    for (const auto guild_id : folder.GuildIDs) {
        if (auto *guild_widget = CreateGuildWidget(guild_id)) {
            guild_widget->show();
            folder_widget->AddGuildWidget(guild_widget);
        }
    }

    folder_widget->show();
    add(*folder_widget);
}

void GuildList::Clear() {
    const auto children = get_children();
    for (auto *child : children) {
        delete child;
    }
}

GuildList::type_signal_guild_selected GuildList::signal_guild_selected() {
    return m_signal_guild_selected;
}

GuildList::type_signal_dms_selected GuildList::signal_dms_selected() {
    return m_signal_dms_selected;
}
