#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED

class Player
{
public:
	Player();
	void update();
	int HP;
	int coin;
	int max_key;
private:
	int coin_freq;
	int coin_increase;
	int coin_counter;
};

#endif
