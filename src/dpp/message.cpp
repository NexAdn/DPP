/************************************************************************************
 *
 * D++, A Lightweight C++ library for Discord
 *
 * Copyright 2021 Craig Edwards and D++ contributors 
 * (https://github.com/brainboxdotcc/DPP/graphs/contributors)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ************************************************************************************/
#include <dpp/message.h>
#include <dpp/user.h>
#include <dpp/channel.h>
#include <dpp/guild.h>
#include <dpp/cache.h>
#include <nlohmann/json.hpp>
#include <dpp/discordevents.h>
#include <dpp/stringops.h>

using json = nlohmann::json;

namespace dpp {

embed::~embed() {
}

embed::embed() {
}

message::message() : id(0), channel_id(0), guild_id(0), author(nullptr), member(nullptr), sent(0), edited(0), flags(0),
	type(mt_default), tts(false), mention_everyone(false), pinned(false), webhook_id(0)
{

}

message::message(snowflake _channel_id, const std::string &_content, message_type t) : message() {
	channel_id = _channel_id;
	content = _content;
	type = t;
}

message::message(const std::string &_content, message_type t) : message() {
	content = _content;
	type = t;
}

message::message(snowflake _channel_id, const embed& _embed) : message() {
	channel_id = _channel_id;
	embeds.push_back(_embed);
}

embed::embed(json* j) : embed() {
	title = StringNotNull(j, "title");
	type = StringNotNull(j, "type");
	description = StringNotNull(j, "description");
	url = StringNotNull(j, "url");
	timestamp = TimestampNotNull(j, "timestamp");
	color = Int32NotNull(j, "color");
	if (j->find("footer") != j->end()) {
		dpp::embed_footer f;
		json& fj = (*j)["footer"];
		f.text = StringNotNull(&fj, "text");
		f.icon_url = StringNotNull(&fj, "icon_url");
		f.proxy_url = StringNotNull(&fj, "proxy_url");
		footer = f;
	}
	std::vector<std::string> type_list = { "image", "video", "thumbnail" };
	for (auto& s : type_list) {
		if (j->find(s) != j->end()) {
			std::optional<embed_image> & curr = image;
			if (s == "image") {
				curr = image;
			} else if (s == "video") {
				curr = video;
			} else if (s == "thumbnail") {
				curr = thumbnail;
			}
			json& fi = (*j)[s];
			dpp::embed_image f;
			f.url = StringNotNull(&fi, "url");
			f.height = StringNotNull(&fi, "height");
			f.width = StringNotNull(&fi, "width");
			f.proxy_url = StringNotNull(&fi, "proxy_url");
			curr = f;
	       	}
	}
	if (j->find("provider") != j->end()) {
		json &p = (*j)["provider"];
		dpp::embed_provider pr;
		pr.name = StringNotNull(&p, "name");
		pr.url = StringNotNull(&p, "url");
		provider = pr;
	}
	if (j->find("author") != j->end()) {
		json &a = (*j)["author"];
		dpp::embed_author au;
		au.name = StringNotNull(&a, "name");
		au.url = StringNotNull(&a, "url");
		au.icon_url = StringNotNull(&a, "icon_url");
		au.proxy_icon_url = StringNotNull(&a, "proxy_icon_url");
		author = au;
	}
	if (j->find("fields") != j->end()) {
		json &fl = (*j)["fields"];
		for (auto & field : fl) {
			embed_field f;
			f.name = StringNotNull(&field, "name");
			f.value = StringNotNull(&field, "value");
			f.is_inline = BoolNotNull(&field, "inline");
			fields.push_back(f);
		}
	}
}

embed& embed::add_field(const std::string& name, const std::string &value, bool is_inline) {
	embed_field f;
	f.name = name;
	f.value = value;
	f.is_inline = is_inline;
	fields.push_back(f);
	return *this;
}

embed& embed::set_author(const std::string& name, const std::string& url, const std::string& icon_url) {
	dpp::embed_author a;
	a.name = name;
	a.url = url;
	a.icon_url = icon_url;
	author = a;
	return *this;
}

embed& embed::set_provider(const std::string& name, const std::string& url) {
	dpp::embed_provider p;
	p.name = name;
	p.url = url;
	provider = p;
	return *this;
}

embed& embed::set_image(const std::string& url) {
	dpp::embed_image i;
	i.url = url;
	image = i;
	return *this;
}

embed& embed::set_video(const std::string& url) {
	dpp::embed_image v;
	v.url = url;
	video = v;
	return *this;
}

embed& embed::set_thumbnail(const std::string& url) {
	dpp::embed_image t;
	t.url = url;
	thumbnail = t;
	return *this;
}

embed& embed::set_title(const std::string &text) {
	title = text;
	return *this;
}

embed& embed::set_description(const std::string &text) {
	description = text;
	return *this;
}

embed& embed::set_color(uint32_t col) {
	color = col;
	return *this;
}

embed& embed::set_url(const std::string &u) {
	url = u;
	return *this;
}

std::string message::build_json(bool with_id) const {
	/* This is the basics. once it works, expand on it. */
	json j({
		{"content", content},
		{"channel_id", channel_id},
		{"tts", tts},
		{"nonce", nonce},
		{"flags", flags},
		{"type", type}
	});
	if (with_id) {
		j["id"] = std::to_string(id);
	}
	if (embeds.size()) {
		for (auto& embed : embeds) {
			json e;
			if (!embed.description.empty())
				e["description"] = embed.description;
			if (!embed.title.empty())
				e["title"] = embed.title;
			if (!embed.url.empty())
				e["url"] = embed.url;
			e["color"] = embed.color;
			if (embed.footer.has_value()) {
				e["footer"]["text"] = embed.footer->text;
				e["footer"]["icon_url"] = embed.footer->icon_url;
			}
			if (embed.image.has_value()) {
				e["image"]["url"] = embed.image->url;
			}
			if (embed.thumbnail.has_value()) {
				e["thumbnail"]["url"] = embed.thumbnail->url;
			}
			if (embed.author.has_value()) {
				e["author"]["name"] = embed.author->name;
				e["author"]["url"] = embed.author->url;
				e["author"]["icon_url"] = embed.author->icon_url;
			}
			if (embed.fields.size()) {
				e["fields"] = json();
				for (auto& field : embed.fields) {
					json f({ {"name", field.name}, {"value", field.value}, {"inline", field.is_inline} });
					e["fields"].push_back(f);
				}
			}

			/* Sending embeds only accepts the first entry */
			j["embed"] = e;
			break;
		}
	}
	return j.dump();
}

bool message::is_crossposted() const {
	return flags & m_crossposted;
}

bool message::is_crosspost() const {
	return flags & m_is_crosspost;

}

bool message::supress_embeds() const {
	return flags & m_supress_embeds;
}

bool message::is_source_message_deleted() const {
	return flags & m_source_message_deleted;
}

bool message::is_urgent() const {
	return flags & m_urgent;
}

bool message::is_ephemeral() const {
	return flags & m_ephemeral;
}

bool message::is_loading() const {
	return flags & m_loading;
}


message& message::fill_from_json(json* d) {
	this->id = SnowflakeNotNull(d, "id");
	this->channel_id = SnowflakeNotNull(d, "channel_id");
	this->guild_id = SnowflakeNotNull(d, "guild_id");
	/* We didn't get a guild id. See if we can find one in the channel */
	if (guild_id == 0 && channel_id != 0) {
		dpp::channel* c = dpp::find_channel(this->channel_id);
		if (c) {
			this->guild_id = c->guild_id;
		}
	}
	this->flags = Int8NotNull(d, "flags");
	this->type = Int8NotNull(d, "type");
	this->author = nullptr;
	user* authoruser = nullptr;
	/* May be null, if its null cache it from the partial */
	if (d->find("author") != d->end()) {
		json &author = (*d)["author"];
		authoruser = find_user(SnowflakeNotNull(&author, "id"));
		if (!authoruser) {
			/* User does not exist yet, cache the partial as a user record */
			authoruser = new user();
			authoruser->fill_from_json(&author);
			get_user_cache()->store(authoruser);
		}
		this->author = authoruser;
	}
	if (d->find("mentions") != d->end()) {
		json &sub = (*d)["mentions"];
		for (auto & m : sub) {
			mentions.push_back(SnowflakeNotNull(&m, "id"));
		}
	}
	if (d->find("mention_roles") != d->end()) {
		json &sub = (*d)["mention_roles"];
		for (auto & m : sub) {
			mention_roles.push_back(from_string<snowflake>(m, std::dec));
		}
	}
	if (d->find("mention_channels") != d->end()) {
		json &sub = (*d)["mention_channels"];
		for (auto & m : sub) {
			mention_channels.push_back(SnowflakeNotNull(&m, "id"));
		}
	}
	/* Fill in member record, cache uncached ones */
	guild* g = find_guild(this->guild_id);
	this->member = nullptr;
	if (g && d->find("member") != d->end()) {
		json& mi = (*d)["member"];
		snowflake uid = SnowflakeNotNull(&mi, "id");
		auto thismember = g->members.find(uid);
		if (thismember == g->members.end()) {
			if (authoruser) {
				guild_member* gm = new guild_member();
				gm->fill_from_json(&mi, g, authoruser);
				g->members[authoruser->id] = gm;
				this->member = gm;
			}
		} else {
			this->member = thismember->second;
		}
	}
	if (d->find("embeds") != d->end()) {
		json & el = (*d)["embeds"];
		for (auto& e : el) {
			this->embeds.push_back(embed(&e));
		}
	}
	this->content = StringNotNull(d, "content");
	this->sent = TimestampNotNull(d, "timestamp");
	this->edited = TimestampNotNull(d, "edited_timestamp");
	this->tts = BoolNotNull(d, "tts");
	this->mention_everyone = BoolNotNull(d, "mention_everyone");
	/* TODO: Populate these */
	/* this->reactions */
	if (((*d)["nonce"]).is_string()) {
		this->nonce = StringNotNull(d, "nonce");
	} else {
		this->nonce = std::to_string(SnowflakeNotNull(d, "nonce"));
	}
	this->pinned = BoolNotNull(d, "pinned");
	this->webhook_id = SnowflakeNotNull(d, "webhook_id");
	return *this;
}

};

