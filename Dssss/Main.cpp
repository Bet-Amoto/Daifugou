# include <Siv3D.hpp>
#include "GameEvent.hpp"
using namespace PlayingCard;

class Player {
public:
	Player(String name) { playername = name; };
	virtual Array<Card> initplayer(const Array<Card>& mycard,int givecards) = 0;
	virtual Array<Card> action(const GameEvent& GameEvent,const Array<Card>& mycard) = 0;
	String getname() const { return playername; }

private:
	String playername;
};

class  Gamemastar {
public:
	Gamemastar(Array<Player*> ps) {
		pack = Pack{ 75, Palette::Red };
		players = ps;
		playercard = Array<Array<Card>>(4, Array<Card>());
		makeyamafuda(true);
		ranking = Array<int32>{ 0,1,2,3 };
		font = Font(20);
	}
	//山札を作る
	void makeyamafuda(bool setblind = false) {
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
	void initgame() {
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
		if (gamecount != 1 and (givecards[0].size()!=2 or not usecard(ranking[0], givecards[0])))throw Error(U"Player0 = init Error");
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
	void progress(){
		countturn++;
		if (nowturn == -1)return;
		if (finish[nowturn] != 0)return;
		Array<Card> action = players[nowturn]->action(events, playercard[nowturn]);
		int e = events.setcard(action);
		if (e == -1 or not usecard(nowturn,action)) {
			finish[nowturn] = -1;
			ranking[4 - countplayer(-1)] = nowturn;
			Print << U"Player{} 反則負け(ランク:{} ターン{})"_fmt(players[nowturn]->getname(),rankname[4 - countplayer(-1)], countturn);
			return;
		}
		if (e == 1)Print << U"スート縛り発生(ターン{})"_fmt(countturn);
		if (e == 2)Print << U"複数枚出し縛り発生(ターン{})"_fmt(countturn);
		if (e == 3)Print << U"階段縛り発生(ターン{})"_fmt(countturn);
		if (e == 4)Print << U"革命発生(ターン{})"_fmt(countturn);
		if (e == 100) {
			Print<< U"Player{} パス(ターン{})"_fmt(players[nowturn]->getname(),countturn);
			passcount++;
		}
		else passcount = 0;
		if (passcount >= countplayer(0)-1) {
			events.restart();
			Print<< U"パスが続いたため場が流れます(ターン{})"_fmt(countturn);
			passcount = 0;
		}
		if (playercard[nowturn].size() <= 0) {
			finish[nowturn] = 1;
			ranking[countplayer(1)-1] = nowturn;
			Print << U"Player{} 勝ち抜け(ランク:{} ターン{})"_fmt(players[nowturn]->getname(),rankname[countplayer(1) - 1], countturn);
		}
		if (events.isyagiri()) {
			Print << U"Player{} 8切り発動(ターン{})"_fmt(players[nowturn]->getname(),countturn);
			nowturn--;
			events.restart();
		}
		if (events.issupesan()) {
			Print << U"Player{} スぺ3返し!(ターン{})"_fmt(players[nowturn]->getname(), countturn);
			nowturn--;
			events.restart();
		}
		nowturn = nextturn(nowturn);
	}
	bool isfinish() {
		return countplayer(0) == 0;
	}
	void draw() const{
		Line(900, 0, 900, 720).draw(Palette::Yellowgreen);
		for (auto p : step(players.size())) {
			font(players[p]->getname()).drawAt(Vec2(100, 150 * p + 150),p==nowturn ? Palette::Lightpink : Palette::Black);
		}
		for (auto p : step(playercard.size())) {
			for (auto c : step(playercard[p].size())) {
				pack(playercard[p][c]).drawAt(Vec2(50*c + 200, 150 * p + 150));
			}
		}
		for (auto p : step(events.topcard().size())) {
			pack(events.topcard()[p]).drawAt(Vec2(1100+30*p,350));
		}
	}
	Array<Player*> getRanking() {
		Array<Player*> p;
		for (auto r : ranking) {
			p << players[r];
		}
		return p;
	}
private:
	int32 nextturn(int32 n) {
		if (countplayer(0) == 0)return -1;
		n = (n + 1) % 4;
		while (finish[n] != 0)n = (n + 1) % 4;
		return n;
	}
	bool ishave(int who, Array<Card> cards) {
		for (auto card : cards) {
			if (not ishave(who, card))return false;
		}
		return true;
	}
	bool ishave(int who, Card card) {
		auto itr = find(playercard[who].begin(), playercard[who].end(), card);
		return itr != playercard[who].end();
	}
	bool usecard(int who,Array<Card> cards) {
		if (not ishave(who, cards))return false;
		for (auto card : cards)usecard(who, card);
		return true;
	}
	bool usecard(int who, Card card) {
		auto itr = find(playercard[who].begin(), playercard[who].end(), card);
		if (itr == playercard[who].end())return false;
		playercard[who].erase(itr);
		return true;
	}
	int32 countplayer(int32 c) {
		int co = 0;
		for (auto f : finish)if (f == c)co++;
		return co;
	}

	Pack pack;
	GameEvent events;
	int32 gamecount = 4;
	int32 setcount = 0;
	int32 nowturn = 0;
	int32 countturn = 0;
	int32 passcount = 0;
	Font font;
	Array<Card> yamafuda;
	Array<Card> blindcard;
	Array<Array<Card>> playercard;
	Array<Player*> players;
	Array<int32> ranking;
	Array<int32> finish;
	const Array<String> rankname{ U"大富豪",U"富豪", U"貧民", U"大貧民" };
};

class randombot :public Player {
public:
	randombot() :Player(U"RandomBot") {};
	Array<Card> initplayer(const Array<Card>& mycard, int givecards) override{
		if (givecards == 2)return Array<Card>{mycard[0], mycard[1]};
		if (givecards == 1)return Array<Card>{mycard[0]};
		return Array<Card>();
	}
	Array<Card> action(const GameEvent& gameevent, const Array<Card>& mycard) override{
		Array<Card> tmp = Array<Card>();
		for (auto p : mycard)if (gameevent.checkcard(p))tmp << p;
		if(tmp.size()==0)return Array<Card>();
		Card n = tmp.choice();
		return Array<Card>{n};
	}
private:
};

class mybot :public Player {
public:
	mybot() :Player(U"MyBot") {};
	Array<Card> initplayer(const Array<Card>& mycard, int givecards) override {
		if (givecards == 2)return Array<Card>{mycard[0], mycard[1]};
		if (givecards == 1)return Array<Card>{mycard[0]};
		return Array<Card>();
	}
	Array<Card> action(const GameEvent& gameevent, const Array<Card>& mycard) override {
		Array<Card> tmp = Array<Card>();
		for (auto p : mycard)if (gameevent.checkcard(p))tmp << p;
		if (tmp.size() == 0)return Array<Card>();
		gameevent.sortcard(tmp);
		Card n = tmp.front();
		return Array<Card>{n};
	}
private:
};

void Main()
{
	Window::Resize(1280, 720);
	Scene::SetBackground(Palette::Darkgreen);
	const Array<String> rankname{ U"大富豪",U"富豪", U"貧民", U"大貧民" };
	Array<Player*> players{ (new mybot()),(new randombot()) ,(new randombot()) ,(new randombot()) };
	Gamemastar mastar(players);
	mastar.initgame();
	const double cooltime = 1;
	double t = 0;
	while (System::Update())
	{
		t += Scene::DeltaTime();
		if (t > cooltime) {
			if (mastar.isfinish()) {
				ClearPrint();
				Print << U"ゲームが終了しました";
				Array<Player*> ranking = mastar.getRanking();
				for (auto p : step(ranking.size())) {
					Print << U"{}:{}"_fmt(rankname[p], ranking[p]->getname());
				}
				mastar.initgame();
				t = 0; continue;

			}
			mastar.progress();
			t = 0;
		}
		mastar.draw();
	}
}
