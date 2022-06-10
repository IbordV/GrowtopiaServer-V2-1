#pragma once
#include "enet/enet.h"
#include <Windows.h>
#include <vector>
#include <chrono>
#include <experimental/filesystem>

inline void dialog_handler(ENetPeer* peer, string cch) {
	if (server_on_freeze or pInfo(peer)->update_req >= 1) return;
	stringstream ss(cch);
	string to = "", btn = "";

	string username = "", password = "", passwordverify = "", email = "";
	bool drop_dialog = false, register_dialog = false, trash_dialog = false, small_lock = false, lock_edit = false, door_edit = false, recieve_acc = false;
	string playerNetID = "", dest_world = "", dest_id = "", door_name = "", door_id = "", password_door = "", door_locked = "";
	int drop_ct = 0, trash_ct = 0, pub_sl = 0, ignore_sl = 0;
	while (getline(ss, to, '\n')) {
		vector<string> infoDat = explode("|", to);
		if (infoDat.size() == 2) {
			if (infoDat.at(0) == "buttonClicked") btn = infoDat.at(1);
			if (infoDat.at(0) == "buttonClicked" and infoDat.at(1) == "creategrowid") register_dialog = true;
			if (infoDat.at(0) == "dialog_name" and infoDat.at(1) == "drop_dialog") drop_dialog = true;
			if (infoDat.at(0) == "dialog_name" and infoDat.at(1) == "trash_dialog") trash_dialog = true;
			if (infoDat.at(0) == "dialog_name" and infoDat.at(1) == "sl_edit") small_lock = true;
			if (infoDat.at(0) == "dialog_name" and infoDat.at(1) == "lock_edit") lock_edit = true;
			if (infoDat.at(0) == "dialog_name" and infoDat.at(1) == "recieve_acc") recieve_acc = true;

			if (register_dialog) {
				if (infoDat.at(0) == "username") username = infoDat.at(1);
				if (infoDat.at(0) == "password") password = infoDat.at(1);
				if (infoDat.at(0) == "passwordverify") passwordverify = infoDat.at(1);
				if (infoDat.at(0) == "email") {
					email = infoDat.at(1);
					if (pInfo(peer)->haveGrowId or not pInfo(peer)->inGame) break;
					player_reg(peer, username, password, passwordverify, email);
					break;
				}
			}
			if (trash_dialog) {
				try {
					if (infoDat.at(0) == "count") {
						if (pInfo(peer)->haveGrowId and pInfo(peer)->inGame and pInfo(peer)->passed_data and not pInfo(peer)->tankIDName.empty() and not pInfo(peer)->lobby_guest_acc and pInfo(peer)->currentWorld != "EXIT") {
							string w_ = pInfo(peer)->currentWorld;
							vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [w_](const World& a) { return a.name == w_; });
							if (p != worlds.end()) {
								int c_inv = 0, found = 0, valid = 1;
								World* world = &worlds.at(p - worlds.begin());
								trash_ct = atoi(infoDat.at(1).c_str()), pInfo(peer)->lasttrash_ct = atoi(infoDat.at(1).c_str());
								if (trash_ct <= 0 or trash_ct > 200 or pInfo(peer)->lasttrash_ct <= 0 or pInfo(peer)->lasttrash_ct >= 201 or pInfo(peer)->lasttrash_id > (int)itemDefs.size()) break;
								for (int i = 0; i < (int)pInfo(peer)->inventory.items.size(); i++) {
									if (pInfo(peer)->inventory.items.at(i).itemID == pInfo(peer)->lasttrash_id) {
										found++, c_inv = pInfo(peer)->inventory.items.at(i).itemCount;
										break;
									}
								}
								if (pInfo(peer)->lasttrash_ct > c_inv or found <= 0) {
									//Auto_ban(peer, 24 * 3, "Exploit!");
									break;
								}
								if (not pInfo(peer)->currentWorld.empty() and pInfo(peer)->currentWorld != "EXIT" and world != NULL and world->name != "EXIT") {
									remove_inv(peer, pInfo(peer)->lasttrash_id, pInfo(peer)->lasttrash_ct);
									Send_::OnPlayPositioned(peer, "audio/trash.wav", pInfo(peer)->netID, false);
									Send_::console_msg(peer, "`o" + to_string(pInfo(peer)->lasttrash_ct) + " `w" + itemDefs.at(pInfo(peer)->lasttrash_id).name + " `otrashed.");
								}
								pInfo(peer)->lasttrash_id = 0, pInfo(peer)->lasttrash_ct = 0;
							}
						}
					}
				}
				catch (out_of_range& e) {
					cout << e.what() << endl;
				}
			}
			if (drop_dialog) {
				try {
					if (infoDat.at(0) == "count") {
						if (pInfo(peer)->haveGrowId and pInfo(peer)->inGame and pInfo(peer)->passed_data and not pInfo(peer)->tankIDName.empty() and not pInfo(peer)->lobby_guest_acc and pInfo(peer)->currentWorld != "EXIT") {
							string w_ = pInfo(peer)->currentWorld;
							vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [w_](const World& a) { return a.name == w_; });
							if (p != worlds.end()) {
								int c_inv = 0, found = 0, valid = 1;
								World* world = &worlds.at(p - worlds.begin());
								drop_ct = atoi(infoDat.at(1).c_str()), pInfo(peer)->lastdrop_ct = atoi(infoDat.at(1).c_str());
								if (drop_ct <= 0 or drop_ct > 200 or pInfo(peer)->lastdrop_ct <= 0 or pInfo(peer)->lastdrop_ct >= 201 or pInfo(peer)->lastdrop_id > (int)itemDefs.size()) break;
								for (int i = 0; i < (int)pInfo(peer)->inventory.items.size(); i++) {
									if (pInfo(peer)->inventory.items.at(i).itemID == pInfo(peer)->lastdrop_id) {
										found++, c_inv = pInfo(peer)->inventory.items.at(i).itemCount;
										break;
									}
								}
								if (pInfo(peer)->lastdrop_ct > c_inv or found <= 0) {
									//Auto_ban(peer, 24 * 3, "Exploit!");
									break;
								}
								int loc_x = pInfo(peer)->x + (32 * (pInfo(peer)->isRotatedLeft ? -1 : 1)) + (rand() % 16), loc_y = pInfo(peer)->y + (rand() % 16);
								if (not pInfo(peer)->isRotatedLeft) loc_x = pInfo(peer)->x + (32 * (pInfo(peer)->isRotatedLeft ? -1 : 1)) - (rand() % 16);
								if (loc_x < 0 or loc_y < 0 or pInfo(peer)->x < 0 or pInfo(peer)->y < 0) break;
								if (not pInfo(peer)->currentWorld.empty() and pInfo(peer)->currentWorld != "EXIT" and world != NULL and world->name != "EXIT") {
									gamepacket_t p(180);
									p.Insert("OnTalkBubble");
									p.Insert(pInfo(peer)->netID);
									if (pInfo(peer)->isInv or pInfo(peer)->canWalkInBlocks) {
										p.Insert("Cannot drop while invisible / noclip."), valid = 0;
									} else if (not world->allow_drp and not isWorldOwner(peer, world) and not isWorldAdmin(peer, world) and world->owner_name != "") {
										p.Insert("Owner has disable Drop Items permission!"), valid = 0;
									} else if (world->items[((pInfo(peer)->x + (32 * (pInfo(peer)->isRotatedLeft ? -1 : 1)))/32) + ((loc_y/32) * world->width)].foreground == 6) {
										p.Insert("You can't drop items on the white door."), valid = 0;
									} else if (not check_fg(peer, world, pInfo(peer)->x + (32 * (pInfo(peer)->isRotatedLeft ? -1 : 1)), loc_y)) {
										p.Insert("You can't drop that here, face somewhere with open space."), valid = 0;
									} if (valid <= 0) {
										pInfo(peer)->lastdrop_id = 0, pInfo(peer)->lastdrop_ct = 0;
										p.CreatePacket(peer);
										break;
									}
									remove_inv(peer, pInfo(peer)->lastdrop_id, pInfo(peer)->lastdrop_ct);
									dropitem(peer, world, -1, (float)loc_x, (float)loc_y, pInfo(peer)->lastdrop_id, pInfo(peer)->lastdrop_ct, 0);
								}
								pInfo(peer)->lastdrop_id = 0, pInfo(peer)->lastdrop_ct = 0;
							}
						}
					}
				}
				catch (out_of_range& e) {
					cout << e.what() << endl;
				}
			}
			if (small_lock) {
				if (pInfo(peer)->haveGrowId and pInfo(peer)->inGame and pInfo(peer)->passed_data and not pInfo(peer)->tankIDName.empty() and not pInfo(peer)->lobby_guest_acc and pInfo(peer)->currentWorld != "EXIT") {
					string w_ = pInfo(peer)->currentWorld;
					vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [w_](const World& a) { return a.name == w_; });
					if (p != worlds.end()) {
						World* world = &worlds.at(p - worlds.begin());
						int x = stoi(explode("|", explode("tilex|", cch).at(1)).at(0)), y = stoi(explode("|", explode("tiley|", cch).at(1)).at(0));
						WorldItem block_ = world->items[x + (y * world->width)];
						if (block_.foreground == 202 or block_.foreground == 204 or block_.foreground == 206) {
							if (pInfo(peer)->displayNameBackup == block_.string_data) { /* remove / add access soon */
								if (infoDat.at(0) == "checkbox_public") {
									if (not block_.opened and infoDat.at(1) == "1") {
										world->items[(x + (y * world->width))].opened = true;
										apply_lock(peer, x, y, world->items[(x + (y * world->width))].foreground, world, world->items[(x + (y * world->width))].empty_air, true, false);
									}
									else if (block_.opened and infoDat.at(1) == "0") {
										world->items[(x + (y * world->width))].opened = false;
										apply_lock(peer, x, y, world->items[(x + (y * world->width))].foreground, world, world->items[(x + (y * world->width))].empty_air, false, false);
									}
								}
								if (infoDat.at(0) == "checkbox_ignore") {
									if (infoDat.at(1) == "1") {
										world->items[x + (y * world->width)].empty_air = true;
									}
									else if (infoDat.at(1) == "0") {
										world->items[x + (y * world->width)].empty_air = false;
									}
								}
								if (btn == "re_apply") {
									if (world->items[x + (y * world->width)].empty_air) {
										apply_lock(peer, x, y, world->items[(x + (y * world->width))].foreground, world, true, false, false);
									} else {
										apply_lock(peer, x, y, world->items[(x + (y * world->width))].foreground, world, false, false, false);
									}
									Send_::OnPlayPositioned(peer, "audio/use_lock.wav", pInfo(peer)->netID, false);
								}
								save_blockstate(world, x, y);
								for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
									if (net_p->state != ENET_PEER_STATE_CONNECTED or net_p->data == NULL) continue;
									if (isHere(peer, net_p)) {
										PlayerMoving data_{};
										data_.packetType = 5, data_.punchX = x, data_.punchY = y, data_.characterState = 0x8;
										BYTE* raw = packPlayerMoving(&data_, 112);
										BYTE* blc = raw + 56;
										form_visual(blc, world->items[x + (y * world->width)], *world, net_p);
										send_raw(4, raw, 112, 0, net_p, ENET_PACKET_FLAG_RELIABLE);
									}
								}
							}
						}
					}
				}
			}
			if (lock_edit) {
				if (pInfo(peer)->haveGrowId and pInfo(peer)->inGame and pInfo(peer)->passed_data and not pInfo(peer)->tankIDName.empty() and not pInfo(peer)->lobby_guest_acc and pInfo(peer)->currentWorld != "EXIT") {
					string w_ = pInfo(peer)->currentWorld;
					vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [w_](const World& a) { return a.name == w_; });
					if (p != worlds.end()) {
						World* world = &worlds.at(p - worlds.begin());
						int x = stoi(explode("|", explode("tilex|", cch).at(1)).at(0)), y = stoi(explode("|", explode("tiley|", cch).at(1)).at(0));
						WorldItem block_ = world->items[x + (y * world->width)];
						if (block_.foreground == 242 or block_.foreground == 1796 or block_.foreground == 7188) {
							if (isWorldOwner(peer, world) or isDev(peer) or isMod(peer)) {
								if (btn == "getKey") {
									if (itemDefs.at(block_.foreground).blockType == BlockTypes::LOCK) {
										if (not isWorldOwner(peer, world)) return;
										bool valid = false;
										search_inv(peer, 1424, 1, valid);
										if (not valid) {
											bool valid2 = save_inv_check(1424, 1, peer, false);
											if (not valid2) return;
										}
									}
								}
								if (infoDat.at(0).substr(0, 9) == "checkbox_") {
									string acc_ = infoDat.at(0).substr(infoDat.at(0).find("_") + 1);
									if (find(world->accessed.begin(), world->accessed.end(), acc_) != world->accessed.end()) {
										if (infoDat.at(1) == "0") {
											world->accessed.erase(remove(world->accessed.begin(), world->accessed.end(), acc_), world->accessed.end());
											ifstream ply("players/_" + toLowerText(acc_) + ".json");
											json j;
											ply >> j;
											ply.close();
											string fix_name = j["displayNameBackup"];
											for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
												if (net_p->state != ENET_PEER_STATE_CONNECTED or net_p->data == NULL) continue;
												if (isHere(peer, net_p)) {
													Send_::console_msg(net_p, fix_name + " was removed from a " + itemDefs.at(block_.foreground).name + ".");
												}
											}
											for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
												if (net_p->state != ENET_PEER_STATE_CONNECTED or net_p->data == NULL) continue;
												if (isHere(peer, net_p)) {
													if (pInfo(net_p)->rawName == acc_) {
														if (not pInfo(net_p)->isRoledName and not pInfo(net_p)->isBlueName and not pInfo(net_p)->isDrTitle and not pInfo(net_p)->isOfLegend) {
															pInfo(net_p)->displayName = "`w" + pInfo(peer)->displayNameBackup;
															for (ENetPeer* net_p2 = server->peers; net_p2 < &server->peers[server->peerCount]; ++net_p2) {
																if (net_p2->state != ENET_PEER_STATE_CONNECTED or net_p2->data == NULL) continue;
																if (isHere(peer, net_p2)) {
																	Send_::change_name(net_p2, pInfo(net_p)->netID, pInfo(net_p)->displayName);
																}
															}
														}
														Send_::audio_(net_p, "audio/dialog_cancel.wav", 0);
														Send_::console_msg(net_p, "You lost access to a " + itemDefs.at(block_.foreground).name);
														PlayerMoving data_{};
														data_.packetType = 5, data_.punchX = x, data_.punchY = y, data_.characterState = 0x8;
														BYTE* raw = packPlayerMoving(&data_, 112);
														BYTE* blc = raw + 56;
														form_visual(blc, world->items[x + (y * world->width)], *world, net_p);
														send_raw(4, raw, 112, 0, net_p, ENET_PACKET_FLAG_RELIABLE);
														break;
													}
												}
												if (pInfo(net_p)->rawName == acc_) {
													Send_::audio_(net_p, "audio/dialog_cancel.wav", 0);
													Send_::console_msg(net_p, "You lost access to a " + itemDefs.at(block_.foreground).name + " in "+pInfo(peer)->currentWorld+"");
												}
											}
										}
									}
								}
								if (infoDat.at(0) == "playerNetID") {
									playerNetID = infoDat.at(1);
									for (ENetPeer* neet_p = server->peers; neet_p < &server->peers[server->peerCount]; ++neet_p) {
										if (neet_p->state != ENET_PEER_STATE_CONNECTED or neet_p->data == NULL) continue;
										if (isHere(peer, neet_p)) {
											if (pInfo(neet_p)->netID == stoi(playerNetID)) {
												if (pInfo(neet_p)->rawName == pInfo(peer)->rawName) {
													Send_::talk_bubble(peer, pInfo(peer)->netID, "`w" + pInfo(peer)->displayName + "`w already has access to the lock.", 0, true);
													break;
												} if (find(world->accessed.begin(), world->accessed.end(), pInfo(neet_p)->rawName) != world->accessed.end()) {
													Send_::talk_bubble(peer, pInfo(peer)->netID, "`w" + pInfo(neet_p)->displayName + "`w already has access to the lock.", 0, true);
													break;
												} if (world->accessed.size() >= 20) {
													Send_::talk_bubble(peer, pInfo(peer)->netID, "`wWorld has reached access limit!", 0, true);
													break;
												}
												AccessWorldQ acc;
												acc.id = stoi(playerNetID);
												acc.name = pInfo(neet_p)->rawName;
												world->accQueue.push_back(acc);
												pInfo(neet_p)->accwl_confirm = true, pInfo(neet_p)->accwl_x = x, pInfo(neet_p)->accwl_y = y;
												Send_::audio_(neet_p, "audio/secret.wav", 0);
												Send_::console_msg(neet_p, "`2" + pInfo(peer)->displayName + " `owants to add you to a World Lock. Wrench yourself to accept.");
											}
										}
									}
									break;
								}
								if (infoDat.at(0) == "checkbox_public_wl") {
									if (infoDat.at(1) == "1" and not world->isPublic) {
										world->isPublic = true;
										for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
											if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
											if (isHere(peer, currentPeer)) {
												Send_::console_msg(currentPeer, "`oWorld is now `9PUBLIC`o, everyone can place and break!");
												Send_::audio_(currentPeer, "audio/dialog_confirm.wav", 0);
											}
										}
									}
									else if (infoDat.at(1) == "0" and world->isPublic) {
										world->isPublic = false;
										for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
											if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
											if (isHere(peer, currentPeer)) {
												Send_::console_msg(currentPeer, "`oWorld is no longer `4PUBLIC`o, everyone cannot place and break!");
												Send_::audio_(currentPeer, "audio/dialog_cancel.wav", 0);
											}
										}
									}
								}
								if (infoDat.at(0) == "checkbox_allow_drop") {
									if (infoDat.at(1) == "1" and not world->allow_drp) {
										world->allow_drp = true;
										for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
											if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
											if (isHere(peer, currentPeer)) {
												Send_::console_msg(currentPeer, "`oOwner world has `9ALLOW `oDrop Items permission!");
												Send_::audio_(currentPeer, "audio/dialog_confirm.wav", 0);
											}
										}
									}
									else if (infoDat.at(1) == "0" and world->allow_drp) {
										world->allow_drp = false;
										for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
											if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
											if (isHere(peer, currentPeer)) {
												Send_::console_msg(currentPeer, "`oOwner world has `4DISABLE `oDrop Items permission!");
												Send_::audio_(currentPeer, "audio/dialog_cancel.wav", 0);
											}
										}
									}
								}
								if (infoDat.at(0) == "checkbox_disable_music") {
									if (infoDat.at(1) == "1" and not world->disable_music_blocks) {
										world->disable_music_blocks = true;
									}
									else if (infoDat.at(1) == "0" and world->disable_music_blocks) {
										world->disable_music_blocks = false;
									}
								}
								if (infoDat.at(0) == "checkbox_disable_music_render") {
									if (infoDat.at(1) == "1" and not world->make_music_blocks_invisible) {
										world->make_music_blocks_invisible = true;
									}
									else if (infoDat.at(1) == "0" and world->make_music_blocks_invisible) {
										world->make_music_blocks_invisible = false;
									}
								}
								if (infoDat.at(0) == "tempo") {
									if (world->bpm != atoi(infoDat.at(1).c_str())) {
										if (atoi(infoDat.at(1).c_str()) < 20 or atoi(infoDat.at(1).c_str()) > 200) {
											Send_::talk_bubble(peer, pInfo(peer)->netID, "`wTempo must be from 20-200 BPM.", 0, true);
											break;
										}
										world->bpm = atoi(infoDat.at(1).c_str());
									}
								}
								if (infoDat.at(0) == "minimum_entry_level") {
									if (world->w_level != atoi(infoDat.at(1).c_str())) {
										if (atoi(infoDat.at(1).c_str()) < 0 or atoi(infoDat.at(1).c_str()) > 125) {
											Send_::talk_bubble(peer, pInfo(peer)->netID, "`wValue world level can't be lower `20`w or more then `2125`w!", 0, true);
											break;
										}
										world->w_level = atoi(infoDat.at(1).c_str());
										Send_::talk_bubble(peer, pInfo(peer)->netID, "`wMinimum entry level for this world is set to `2Level " + infoDat.at(1) + "``.", 0, true);
									}
								}
								for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
									if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
									if (isHere(peer, currentPeer)) {
										PlayerMoving data_{};
										data_.packetType = 5, data_.punchX = x, data_.punchY = y, data_.characterState = 0x8;
										BYTE* raw = packPlayerMoving(&data_, 112);
										BYTE* blc = raw + 56;
										form_visual(blc, world->items[x + (y * world->width)], *world, currentPeer);
										send_raw(4, raw, 112, 0, currentPeer, ENET_PACKET_FLAG_RELIABLE);
									}
								}
							}
						}
					}
				}
			}
			if (recieve_acc) {
				if (pInfo(peer)->accwl_confirm) {
					string w_ = pInfo(peer)->currentWorld;
					vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [w_](const World& a) { return a.name == w_; });
					if (p != worlds.end()) {
						World* world = &worlds.at(p - worlds.begin());
						for (int i = 0; i < (int)world->accQueue.size(); i++) {
							if (world->accQueue.at(i).id == pInfo(peer)->netID) {
								WorldItem block_ = world->items[pInfo(peer)->accwl_x + (pInfo(peer)->accwl_y * world->width)];
								world->accQueue.erase(world->accQueue.begin() + i), world->accessed.push_back(pInfo(peer)->rawName), pInfo(peer)->accwl_confirm = false;
								PlayerMoving data_{};
								data_.packetType = 5, data_.punchX = pInfo(peer)->accwl_x, data_.punchY = pInfo(peer)->accwl_y, data_.characterState = 0x8;
								BYTE* raw = packPlayerMoving(&data_, 112);
								BYTE* blc = raw + 56;
								form_visual(blc, world->items[pInfo(peer)->accwl_x + (pInfo(peer)->accwl_y * world->width)], *world, peer);
								send_raw(4, raw, 112, 0, peer, ENET_PACKET_FLAG_RELIABLE);
								for (ENetPeer* net_p = server->peers; net_p < &server->peers[server->peerCount]; ++net_p) {
									if (net_p->state != ENET_PEER_STATE_CONNECTED or net_p->data == NULL) continue;
									if (isHere(peer, net_p)) {
										Send_::console_msg(net_p, "`w" + pInfo(peer)->displayNameBackup + " `owas given access to a " + itemDefs.at(block_.foreground).name + ".");
										if (not pInfo(peer)->isRoledName and not pInfo(peer)->isBlueName and not pInfo(peer)->isDrTitle and not pInfo(peer)->isOfLegend) {
											pInfo(peer)->displayName = "`^" + pInfo(peer)->displayNameBackup;
											Send_::change_name(net_p, pInfo(peer)->netID, pInfo(peer)->displayName);
										}
									}
								}
								Send_::audio_(peer, "audio/secret.wav", 0);
								pInfo(peer)->accwl_x = 0, pInfo(peer)->accwl_y = 0;
								break;
							}
						}
					}
				}
			}
		}
	}
	if (btn == "growid") {
		Send_::dialog_(peer, "set_default_color|`o\nadd_label_with_icon|big|`wGet a GrowID``|left|206|\nadd_spacer|small|\nadd_textbox|By choosing a `wGrowID``, you can use a name and password to logon from any device.Your `wname`` will be shown to other players!|left|\nadd_spacer|small|\nadd_text_input|username|Name||18|\nadd_textbox|Your `wpassword`` must contain `w8 to 18 characters, 1 letter, 1 number`` and `w1 special character: @#!$^&*.,``|left|\nadd_text_input_password|password|Password||18|\nadd_text_input_password|passwordverify|Password Verify||18|\nadd_textbox|Your `wemail`` will only be used for account verification and support. If you enter a fake email, you can't verify your account, recover or change your password.|left|\nadd_text_input|email|Email||64|\nadd_textbox|We will never ask you for your password or email, never share it with anyone!|left|\nadd_button|creategrowid|`wGet My GrowID!|\n");
	}
	else if (btn == "accept_acc") {
		if (pInfo(peer)->haveGrowId and pInfo(peer)->inGame and pInfo(peer)->passed_data and not pInfo(peer)->tankIDName.empty() and not pInfo(peer)->lobby_guest_acc and pInfo(peer)->currentWorld != "EXIT") {
			if (pInfo(peer)->accwl_confirm) {
				string w_ = pInfo(peer)->currentWorld;
				vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [w_](const World& a) { return a.name == w_; });
				if (p != worlds.end()) {
					World* world = &worlds.at(p - worlds.begin());
					for (int i = 0; i < (int)world->accQueue.size(); i++) {
						if (world->accQueue.at(i).id == pInfo(peer)->netID) {
							WorldItem block_ = world->items[pInfo(peer)->accwl_x + (pInfo(peer)->accwl_y * world->width)];
							Send_::dialog_(peer, "set_default_color|`o\nadd_label_with_icon|small|Accept Access To World|left|" + to_string(itemDefs.at(block_.foreground).id) + "|\nadd_smalltext|When you have access to a world, you are responsible for anything the world's owners or other admins do.|left|\nadd_smalltext|Don't accept access to a world unless it is from people you trust.|left|\nadd_smalltext|You can remove your access later by either wrenching the lock, or typing `2/unaccess`` to remove yourself from all locks in the world.|left|\nadd_spacer|small|\nadd_textbox|Are you sure you want to be added to this " + itemDefs.at(block_.foreground).name + "?|left|\nend_dialog|recieve_acc|`wNo|`wYes|");
							break;
						}
					}
				}
			}
		}
	}
}