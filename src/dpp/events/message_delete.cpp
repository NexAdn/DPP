#include <dpp/discord.h>
#include <dpp/event.h>
#include <string>
#include <iostream>
#include <fstream>
#include <dpp/discordclient.h>
#include <dpp/discord.h>
#include <dpp/cache.h>
#include <dpp/stringops.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/**
 * @brief Handle event
 * 
 * @param client Websocket client (current shard)
 * @param j JSON data for the event
 * @param raw Raw JSON string
 */
void message_delete::handle(class DiscordClient* client, json &j, const std::string &raw) {
	if (client->creator->dispatch.message_delete) {
		json d = j["d"];
		dpp::message_delete_t msg(raw);
		dpp::message m;
		m.fill_from_json(&d);
		msg.deleted = &m;
		client->creator->dispatch.message_delete(msg);
	}

}

