# include <Siv3D.hpp>
#include "GameEvent.hpp"
#include "Player.hpp"
#include "Gamemaster.hpp"
using namespace PlayingCard;

//反則しない程度にランダムに打つ
class randombot :public Player {
public:
	//Player()のところでこのbotの名前を設定
	randombot() :Player(U"RandomBot") {};

	Array<Card> initplayer(const Array<Card>& mycard, int givecards) override{
		//貧民、大貧民の時はゲームマスターがかってに強い順にカードを選択するのでgivecardsの値は0になります
		//渡すカードが2枚なら弱いほうから2枚選択
		if (givecards == 2)return Array<Card>{mycard[0], mycard[1]};
		//渡すカードが1枚なら弱いほうから1枚選択
		if (givecards == 1)return Array<Card>{mycard[0]};
		//渡すカードが0なら空の配列を返す
		return Array<Card>();
	}
	Array<Card> action(const GameEvent& gameevent, const Array<Card>& mycard) override{
		Array<Card> tmp = Array<Card>();
		//場に出せるカードを検索
		for (auto p : mycard)if (gameevent.checkcard(p))tmp << p;
		//出せるカードがなければパス
		if(tmp.size()==0)return Array<Card>();
		//出せるカードの中からランダムに1枚カードを選ぶ
		Card n = tmp.choice();
		//選択したカードを返す
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
	//ここでプレイヤーの登録
	Array<Player*> players{ (new mybot()),(new randombot()) ,(new randombot()) ,(new randombot()) };

	//ここの数字を小さくすると試合の進行が速くなるよ
	const double cooltime = 0.2;

	//2つめの引数の数字を変えるとlogの量を調整できるよ(0-3) 0はすべてのlog、3はlogなし
	Gamemastar mastar(players,3);

	//爆速モード 超高速で5000ゲーム繰り返す logの設定は3推奨
	const bool bakusoku = true;

	Window::Resize(1280, 720);
	Scene::SetBackground(Palette::Darkgreen);
	const Array<String> rankname{ U"大富豪",U"富豪", U"貧民", U"大貧民" };
	const Array<int32> rankscore{ 6, 4, 2, 0 };
	Array<int32> playerscore(4,0);
	mastar.initgame();
	double t = 0;
	int32 gamecount = 0;
	while (System::Update())
	{

		if (bakusoku) {
			if (gamecount > 5000)continue;
			String logstr;
			for (auto _ : step(4)) {
				while (not mastar.isfinish()) {
					mastar.progress();
				}
				logstr = U"";
				Array<Player*> ranking = mastar.getRanking();
				Array<String> tmpp(4, U"");
				for (auto p : step(ranking.size())) {
					auto itr = find(players.begin(), players.end(), ranking[p]);
					int32 who = distance(players.begin(), itr);
					playerscore[who] += rankscore[p];
					tmpp[who] = U"{}:{}  "_fmt(ranking[p]->getname(), playerscore[who]);
				}
				for (auto s : tmpp)logstr += s;
				if (gamecount % 100 == 0)Logger << U"game{}... "_fmt(gamecount) + logstr;
				mastar.initgame();
				gamecount++;
			}
			ClearPrint();
			if (gamecount > 5000) {
				Logger << U"Finish!";
				Print << U"Finish!";
				Print << logstr;
			}
			else {
				Print << U"ゲーム数:{}"_fmt(gamecount);
			}
			continue;
		}

		t += Scene::DeltaTime();
		if (t > cooltime) {
			if (mastar.isfinish()) {
				ClearPrint();
				Print << U"ゲームが終了しました";
				Array<Player*> ranking = mastar.getRanking();
				for (auto p : step(ranking.size())) {
					auto itr = find(players.begin(), players.end(), ranking[p]);
					int32 who = distance(players.begin(), itr);
					playerscore[who] += rankscore[p];
					Print << U"{}:{}(現在{}点)"_fmt(rankname[p], ranking[p]->getname(),playerscore[who]);
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
