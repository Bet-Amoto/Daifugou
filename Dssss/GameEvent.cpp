#include "GameEvent.hpp"

GameEvent::GameEvent() { reset(); }
void GameEvent::reset() {
	opencards.clear();
	topcards.clear();
	suittight = false;
	kaidantight = false;
	multitight = false;
	kakumei = false;
	nocard = true;
	supesan = false;
	yagiri = false;
}
void GameEvent::restart() {
	topcards.clear();
	suittight = false;
	kaidantight = false;
	multitight = false;
	nocard = true;
	supesan = false;
	yagiri = false;
}
bool GameEvent::isrestart() const { return nocard; }
//スート縛りがあるか
bool GameEvent::issuittight() const { return suittight; }
//階段縛りがあるか
bool GameEvent::iskaidantight() const { return kaidantight; }
//複数枚縛りがあるか
bool GameEvent::ismultitight() const { return multitight; }
//革命起きてるか
bool GameEvent::iskakumei() const { return kakumei; }
bool GameEvent::isyagiri() { return yagiri; }
bool GameEvent::issupesan() { return supesan; }
//今の一番上にアルカード
Array<Card> GameEvent::topcard() const { return topcards; }
//これまでに場に出されたカード
Array<Card> GameEvent::openedcards() const { return opencards; }
bool GameEvent::checkcard(Array<Card> setcard) const {
	sortcard(setcard);
	//場にカードがなければtrue
	if (isrestart())return true;
	//カードの枚数が違ったらfalse
	if (topcard().size() != setcard.size())return false;
	//スぺ3返しでtrue
	if (setcard.size() == 1 and ((setcard[0].isSpade() and setcard[0].rank == 3) and topcard()[0].isJoker()))return true;
	//一番弱いカード同士を比較してsetcardが弱かったらfalse
	if (not compareStrange(setcard[0], topcard()[0]))return false;
	//階段縛りで階段じゃなかったらfalse
	if (iskaidantight() and not iskaidan(setcard))return false;
	//複数枚縛り守らなかったらfalse
	if (ismultitight() and (not ismulti(setcard) or setcard[0].rank <= topcard()[0].rank))return false;
	//スート縛り守らなかったらfalse
	if (issuittight() and not comparesuit(setcard, topcard())) return false;

	return true;
}
bool GameEvent::checkcard(Card setcard) const { Array<Card> tmp{ setcard }; return checkcard(tmp); }
int GameEvent::setcard(Array<Card> setcard) {
	if (setcard.size() == 0)return 100;
	sortcard(setcard);
	if (not checkcard(setcard))return -1;
	int ev = 0;
	if (isrestart()) {
		if (setcard.size() >= 2) {
			//階段なら縛り発動
			if (iskaidan(setcard)) {
				kaidantight = true;
				ev = 3;
			}
			//複数枚出すと縛り発動
			if (ismulti(setcard)) {
				multitight = true;
				ev = 2;
				//4枚以上同じで革命
				if (setcard.size() >= 4) {
					kakumei = true;
					sortcard(setcard);
					ev = 4;
				}
			}
		}
		nocard = false;
	}
	//前のカードとスートが同じなら縛り発動
	else {
		if (comparesuit(topcards, setcard, true) and not issuittight()) {
			suittight = true;
			ev = 1;
		}
		if (topcard().size() == 1 && topcard()[0].suit == Suit::Joker && setcard[0].rank == 3 && setcard[0].suit == Suit::Spade)supesan = true;
	}

	if (not iskaidantight() and setcard[0].rank == 8) yagiri = true;
	topcards = setcard;
	opencards.insert(opencards.end(), setcard.begin(), setcard.end());
	return ev;
}
//左が強ければtrue
bool GameEvent::compareStrange(Card a, Card b) const {
	if (a.isJoker())return true;
	if (b.isJoker())return false;
	if (a.rank == 1 || a.rank == 2)a.rank += 13;
	if (b.rank == 1 || b.rank == 2)b.rank += 13;
	return iskakumei() ? a.rank < b.rank : a.rank>b.rank;
}
//カードを弱い順にソート
void GameEvent::sortcard(Array<Card>& cards) const {
	if (not iskakumei()) {
		cards.sort_by([](Card a, Card b) {
			if (a.isJoker())return false;
			if (b.isJoker())return true;
			if (a.rank == 1 || a.rank == 2)a.rank += 13;
			if (b.rank == 1 || b.rank == 2)b.rank += 13;
			return a.rank < b.rank; });
	}
	else {
		cards.sort_by([](Card a, Card b) {
			if (a.isJoker())return false;
			if (b.isJoker())return true;
			if (a.rank == 1 || a.rank == 2)a.rank += 13;
			if (b.rank == 1 || b.rank == 2)b.rank += 13;
			return a.rank > b.rank; });
	}
}
//カードのスートの比較
bool GameEvent::comparesuit(Array<Card> a, Array<Card> b, bool nojoker) const {
	sortcard(a);
	sortcard(b);
	size_t jokersA = a.size();
	a.remove_if([](Card x) {return x.isJoker(); });
	jokersA -= a.size();
	if (nojoker and jokersA > 0)return false;
	size_t jokersB = b.size();
	b.remove_if([](Card x) {return x.isJoker(); });
	jokersB -= b.size();
	if (nojoker and jokersB > 0)return false;
	a.sort_by([](Card x, Card y) {return x.suit < y.suit; });
	b.sort_by([](Card x, Card y) {return x.suit < y.suit; });
	int32 i = 0, j = 0;
	while (i < a.size() && j < b.size()) {
		if (a[i].suit < b[j].suit) {
			if (jokersB > 0) {
				jokersB--;
				i++;
			}
			else return false;
		}
		else if (a[i].suit > b[j].suit) {
			if (jokersA > 0) {
				jokersA--;
				j++;
			}
			else return false;
		}
		else {
			j++;
			i++;
		}
	}
	return true;
}
//全部同じ数字か
bool GameEvent::ismulti(Array<Card> cards) const {
	if (cards.size() < 2)return false;
	sortcard(cards);
	int num = cards[0].rank;
	for (auto card : cards) {
		if (not num == card.rank and not card.isJoker())return false;
	}
	return true;
}
//階段になってるか
bool GameEvent::iskaidan(Array<Card> cards) const {
	if (cards.size() < 3)return false;
	sortcard(cards);
	int32 jokers = (int32)cards.size();
	cards.remove_if([](Card a) {return a.isJoker(); });
	jokers -= (int32)cards.size();
	int32 suits = cards[0].suit;
	for (auto i : step(cards.size() - 1)) {
		if (cards[i + 1].suit != suits)return false;
		if (cards[i + 1].rank - cards[i].rank != 1) {
			if (jokers <= 0)return false;
			jokers--;
			cards[i].rank++;
			i--;
		}
	}
	return true;
}
