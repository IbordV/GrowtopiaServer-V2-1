#pragma once
#include "enet/enet.h"
#include <Windows.h>
#include <vector>
#include <chrono>
#include <experimental/filesystem>

inline void command_process(ENetPeer* peer, string cch) {
	if (pInfo(peer)->haveGrowId and pInfo(peer)->inGame and pInfo(peer)->passed_data and not pInfo(peer)->tankIDName.empty() and not pInfo(peer)->lobby_guest_acc and pInfo(peer)->currentWorld != "EXIT") {
		string w_ = pInfo(peer)->currentWorld;
		vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [w_](const World& a) { return a.name == w_; });
		if (p != worlds.end()) {
			World* world = &worlds.at(p - worlds.begin());
			if (cch.substr(0, 6) == "/item ") {
				if (isDev(peer)) {
					int iteee = atoi(cch.substr(6).c_str());
					if (iteee <= 0) return;
					InventoryItem item;
					item.itemID = atoi(cch.substr(6).c_str());
					item.itemCount = 200;
					pInfo(peer)->inventory.items.push_back(item);
					send_inv(peer, pInfo(peer)->inventory);
					return;
				}
				Send_::console_msg(peer, "`4Unknown command. `oEnter `$/help `ofor a list of valid commands.");
			}
		}
	}
}