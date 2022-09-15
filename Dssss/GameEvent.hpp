#pragma once
# include <Siv3D.hpp>
using namespace PlayingCard;

class GameEvent {
public:

	//場にカードがなければtrue
	bool isrestart() const;

	//スート縛りがあるか
	bool issuittight() const;

	//階段縛りがあるか
	bool iskaidantight() const;

	//複数枚縛りがあるか
	bool ismultitight() const;

	//革命起きてるか
	bool iskakumei() const;

	//今一番上にあるカード
	Array<Card> topcard() const;

	//これまでに場に出されたカード
	Array<Card> openedcards() const;

	//そのカードが出せるか
	bool checkcard(Array<Card> setcard) const;
	bool checkcard(Card setcard) const;

	//左が強ければtrue
	bool compareStrange(Card a, Card b) const;

	//カードを弱い順にソート
	void sortcard(Array<Card>& cards) const;

	//カードのスートの比較
	bool comparesuit(Array<Card> a, Array<Card> b, bool nojoker = false) const;

	//全部同じ数字か
	bool ismulti(Array<Card> cards) const;

	//階段になってるか
	bool iskaidan(Array<Card> cards) const;

	////////////////////////////////////////////////////////////////////////////////////

	GameEvent();
	//ゲーム開始時に呼ぶ
	void reset();
	//場を流す
	void restart();
	//8切りされたか
	bool isyagiri();
	//スぺ3返しがおきたか
	bool issupesan();
	//@return -1...エラー 0...通常 1...スート縛り発生 2...複数枚縛り発生 3...階段縛り発生 4...革命発生 100...パス
	int setcard(Array<Card> setcard);
private:
	bool nocard;
	bool suittight;
	bool kaidantight;
	bool multitight;
	bool kakumei;
	bool yagiri;
	bool supesan;
	Array<Card> topcards;
	Array<Card> opencards;

};
