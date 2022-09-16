#include "Gamemaster.hpp"
Gamemastar::Gamemastar(Array<Player*> ps,int32 log) {
	pack = Pack{ 75, Palette::Red };
	players = ps;
	playercard = Array<Array<Card>>(4, Array<Card>());
	makeyamafuda(true);
	ranking = Array<int32>{ 0,1,2,3 };
	font = Font(20);
	loglv = log;
}
//山札を作る
void Gamemastar::makeyamafuda(bool setblind) {
	if (not setblind) {
		yamafuda.shuffle();
		events = GameEvent();
		return;
	}
	//公式ルールに基づいてシャッフル
	Array<Card> cards = CreateDeck(0);
	blindcard.clear();
	cards.shuffle();
	blindcard << cards.back();
	cards.pop_back();
	blindcard << cards.back();
	cards.pop_back();
	cards << Card(Suit::Joker, 50) << Card(Suit::Joker, 50);
	cards.shuffle();
	yamafuda = cards;
}
void Gamemastar::initgame() {
	gamecount++;
	countturn = 0;
	if (gamecount > 4) {
		setcount++;
		gamecount = 1;
		makeyamafuda(true);
		players.shuffle();
		ranking = Array<int32>{ 0,1,2,3 };
		nowturn = 0;
	}
	else {
		makeyamafuda(false);
		nowturn = ranking[3];
	}
	finish = Array<int32>(4, 0);
	events.reset();
	for (auto& pc : playercard)pc.clear();
	//手札配給
	for (auto i : step(13)) {
		playercard[0] << yamafuda[4 * i];
		playercard[1] << yamafuda[4 * i + 1];
		playercard[2] << yamafuda[4 * i + 2];
		playercard[3] << yamafuda[4 * i + 3];
	}

	//全員の手札を弱い順にソート
	for (auto& pc : playercard)events.sortcard(pc);

	//順位でカード交換
	Array<Array<Card>> givecards(4, Array<Card>());
	givecards[0] = players[ranking[0]]->initplayer(playercard[ranking[0]], gamecount == 1 ? 0 : 2);
	if (gamecount != 1 and (givecards[0].size() != 2 or not usecard(ranking[0], givecards[0])))throw Error(U"Player0 = init Error");
	givecards[1] = players[ranking[1]]->initplayer(playercard[ranking[1]], gamecount == 1 ? 0 : 1);
	if (gamecount != 1 and (givecards[1].size() != 1 or not usecard(ranking[1], givecards[1])))throw Error(U"Player1 = init Error");
	players[ranking[2]]->initplayer(playercard[ranking[2]], 0);
	players[ranking[3]]->initplayer(playercard[ranking[3]], 0);
	if (gamecount != 1) {
		givecards[2] << playercard[ranking[2]].back();
		usecard(ranking[2], playercard[ranking[2]].back());
		givecards[3] << playercard[ranking[3]].back();
		usecard(ranking[3], playercard[ranking[3]].back());
		givecards[3] << playercard[ranking[3]].back();
		usecard(ranking[3], playercard[ranking[3]].back());
		for (auto i : step(4)) playercard[ranking[i]].append(givecards[3 - i]);
	}
}
void Gamemastar::progress() {
	countturn++;
	if (nowturn == -1)return;
	if (finish[nowturn] != 0)return;
	Array<Card> action = players[nowturn]->action(events, playercard[nowturn]);
	int e = events.setcard(action);
	if (e == -1 or not usecard(nowturn, action)) {
		finish[nowturn] = -1000+nowturn;
		printlog( U"Player{} 反則負け(ターン{})(カード{}:{})"_fmt(players[nowturn]->getname(), countturn, action[0].suit,action[0].rank), 3);
	}
	if (e == 1)printlog(U"スート縛り発生(ターン{})"_fmt(countturn), 2);
	if (e == 2)printlog(U"複数枚出し縛り発生(ターン{})"_fmt(countturn), 2);
	if (e == 3)printlog(U"階段縛り発生(ターン{})"_fmt(countturn), 2);
	if (e == 4)printlog(U"革命発生(ターン{})"_fmt(countturn), 2);
	if (e == 100) {
		printlog(U"Player{} パス(ターン{})"_fmt(players[nowturn]->getname(), countturn));
		passcount++;
	}
	else passcount = 0;
	if (passcount >= countplayer(0) - 1) {
		events.restart();
		printlog(U"パスが続いたため場が流れます(ターン{})"_fmt(countturn));
		passcount = 0;
	}
	if (playercard[nowturn].size() <= 0) {
		if (events.ishansoku(action)) {
			finish[nowturn] = -1000 + countturn;
			printlog(U"Player{} 反則上がり!(ターン{})(カード{}:{})"_fmt(players[nowturn]->getname(), countturn, action[0].suit, action[0].rank), 3);
		}
		else {
			finish[nowturn] = 1000 - countturn;
			printlog( U"Player{} 勝ち抜け(ターン{})"_fmt(players[nowturn]->getname(), countturn), 3);
			if (gamecount != 1 and finish[ranking[0]] == 0) {
				printlog(U"Player{} 都落としにより脱落(ターン{})"_fmt(players[ranking[0]]->getname(), countturn), 3);
				finish[ranking[0]] = -10;
			}
		}
	}
	if (events.isyagiri()) {
		printlog(U"Player{} 8切り発動(ターン{})"_fmt(players[nowturn]->getname(), countturn),2);
		nowturn--;
		events.restart();
	}
	if (events.issupesan()) {
		printlog(U"Player{} スぺ3返し!(ターン{})"_fmt(players[nowturn]->getname(), countturn),2);
		nowturn--;
		events.restart();
	}
	nowturn = nextturn(nowturn);
	if (isfinish())ranking = setranking(finish);
}
bool Gamemastar::isfinish() {
	return countplayer(0) == 0;
}
void Gamemastar::draw() const {
	Line(900, 0, 900, 720).draw(Palette::Yellowgreen);
	for (auto p : step(players.size())) {
		font(players[p]->getname()).drawAt(Vec2(100, 150 * p + 150), p == nowturn ? Palette::Lightpink : Palette::Black);
	}
	for (auto p : step(playercard.size())) {
		for (auto c : step(playercard[p].size())) {
			pack(playercard[p][c]).drawAt(Vec2(50 * c + 200, 150 * p + 150));
		}
	}
	for (auto p : step(events.topcard().size())) {
		pack(events.topcard()[p]).drawAt(Vec2(1100 + 30 * p, 350));
	}
}
Array<Player*> Gamemastar::getRanking() {
	Array<Player*> p;
	for (auto r : ranking) {
		p << players[r];
	}
	return p;
}
int32  Gamemastar::nextturn(int32 n) {
	if (countplayer(0) == 0)return -1;
	n = (n + 1) % 4;
	while (finish[n] != 0)n = (n + 1) % 4;
	return n;
}
bool  Gamemastar::ishave(int who, Array<Card> cards) {
	for (auto card : cards) {
		if (not ishave(who, card))return false;
	}
	return true;
}
bool  Gamemastar::ishave(int who, Card card) {
	auto itr = find(playercard[who].begin(), playercard[who].end(), card);
	return itr != playercard[who].end();
}
bool  Gamemastar::usecard(int who, Array<Card> cards) {
	if (not ishave(who, cards))return false;
	for (auto card : cards)usecard(who, card);
	return true;
}
bool  Gamemastar::usecard(int who, Card card) {
	auto itr = find(playercard[who].begin(), playercard[who].end(), card);
	if (itr == playercard[who].end())return false;
	playercard[who].erase(itr);
	return true;
}
Array<int32> Gamemastar::setranking(Array<int32> fin) {
	int32 hiscore = -9999;
	int32 pos = 0;
	Array<int32> ran;
	for (auto i : step(4)) {
		hiscore = -9999;
		for (auto j : step(4)) {
			if (find(ran.begin(), ran.end(), j) != ran.end())continue;
			if (fin[j] > hiscore) {
				hiscore = fin[j];
				pos = j;
			}
		}
		ran << pos;
	}
	return ran;
}
void Gamemastar::printlog(String str, int32 logLv) {
	if (logLv > loglv) {
		Print << str;
		Logger << str;
	}
}
int32  Gamemastar::countplayer(int32 c) {
	int co = 0;
	for (auto f : finish)if (f == c)co++;
	return co;
}
