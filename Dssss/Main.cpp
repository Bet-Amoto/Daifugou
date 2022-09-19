# include <Siv3D.hpp>
#include "GameEvent.hpp"
#include "Player.hpp"
#include "Gamemaster.hpp"
using namespace PlayingCard;

//ランダムにカードを決定
class randombot :public Player {
public:
	//Player()のところでこのbotの名前を設定
	randombot() :Player(U"RandomBot{}"_fmt(Random(10,99))) {};

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
		for (const auto& p : mycard)if (gameevent.checkcard(p))tmp << p;
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
		for (const auto& p : mycard)if (gameevent.checkcard(p))tmp << p;
		if (tmp.size() == 0)return Array<Card>();
		gameevent.sortcard(tmp);
		Card n = tmp.front();
		return Array<Card>{n};
	}
private:
};

//imageに文字列を書き込む関数(ほぼチュートリアルのコピペ)
Vec2  drawstr(Image& image,const Font& font ,const String& str,Vec2 pos,const ColorF& col = Palette::Black) {
	for (const auto& ch : str)
	{
		const BitmapGlyph bitmapGlyph = font.renderBitmap(ch);

		// 文字のテクスチャをペンの位置に文字ごとのオフセットを加算して描画
		// .asPoint() は Vec2 を Point に変換する関数
		bitmapGlyph.image.paint(image, (pos + bitmapGlyph.getOffset()).asPoint(), col);

		// ペンの X 座標を文字の幅の分進める
		pos.x += bitmapGlyph.xAdvance;
	}
	//文字列を書いた後のペンの座標を返す
	return pos;
}

void Main()
{
	//ここでプレイヤーの登録
	Array<Player*> players{ (new mybot()),(new randombot()) ,(new randombot()) ,(new randombot()) };

	//ここの数字を小さくすると試合の進行が速くなるよ(爆速モード未使用時)
	const double cooltime = 0.5;

	//2つめの引数の数字を変えるとlogの量を調整できるよ(0-3) 0はすべてのlogを標示、3はlogなし
	Gamemastar mastar(players, 1);

	//爆速モード 超高速で5000ゲーム繰り返す logの設定は3推奨
	const bool bakusoku = false;

	//保存するグラフの画像名(爆速モード有効時のみ使用)
	const String graphname = U"test.png";

	//グラフのプレイヤーの色　playersに登録した順で
	const Array<ColorF> playerColor{ ColorF(1,0,0),ColorF(0,1,0) ,ColorF(0,0,1) ,ColorF(0.5,0.5,0) };

	Window::Resize(1280, 720);
	Scene::SetBackground(Palette::Darkgreen);
	const Array<String> rankname{ U"大富豪",U"富豪", U"貧民", U"大貧民" };
	const Array<int32> rankscore{ 6, 4, 2, 0 };
	Array<int32> playerscore(4,0);
	double t = 0;

	const Size size{ 800,700 };
	const Font font{ 25 };
	Image image{ size, Palette::White };
	int32 gamecount = 0;
	constexpr Vec2 basePos{ 20, 650 };
	Vec2 penPos{ basePos };
	Line(150, 100, 150, 600).overwrite(image, 3, Palette::Black);
	Line(150, 600, 750, 600).overwrite(image, 3, Palette::Black);
	drawstr(image, font, U"0", Vec2(130,600));
	for (auto i : step(4)) {
		Line(150, 100 + (i + 1) * 100, 750, 100 + (i + 1) * 100).overwrite(image, 1, Palette::Black);
		String sc = U"{:>7}"_fmt(6000 * (4 - i));
		drawstr(image, font,sc, Vec2(50,180+100*i));
	}
	for (auto i : step(5)) {
		Line(150 + (i + 1) * 100, 100, 150 + (i + 1) * 100, 600).overwrite(image, 1, Palette::Black);
		String sc = U"{}"_fmt(1000 * (i + 1));
		drawstr(image, font, sc, Vec2(220 + i * 100, 600));
	}
	for (const auto& p : step(players.size())) {
		penPos = drawstr(image, font, players[p]->getname(), penPos, playerColor[p]);
		penPos.x += 30;
	}
	DynamicTexture texture{ image };
	bool save = true;

	mastar.initgame();

	while (System::Update())
	{
		//爆速モードの時の処理
		if (bakusoku) {
			if (gamecount > 5000) {
				if (save) {
					save = false;
					image.save(graphname);
				}
				texture.draw();
				continue;
			}
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
					Circle(150+(double)gamecount/5000*500,600-(double)playerscore[who]/30000*500, 2).overwrite(image, playerColor[who]);
				}
				for (const auto& s : tmpp)logstr += s;
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
			texture.fill(image);
			texture.draw();

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
