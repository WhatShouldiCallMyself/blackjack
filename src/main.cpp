#include <iostream>
#include <optional>
#include <stdlib.h>
#include <time.h>
#include <tuple>
#include <vector>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

#define INTRO_MSG "Welcome to a game of Blackjack!\n"
#define OUTRO_MSG "Thanks for playing!\n"
#define PLAYER_TURN_MSG "Please choose your next move.\n1 = Hit, 2 = Stand, 3 = Fold\nYour current deck: " 

void swap(char* a, char* b) {
	char temp = *a;
	*a = *b;
	*b = temp;
}

void shuffle(std::vector<char>& arr, const int& n) {
	srand(time(NULL));

	for (int i = n - 1; i > 0; i--) {
		int j = rand() % (i + 1);
		swap(&arr[i], &arr[j]);
	}
}

int get_char() {
	#ifdef _WIN32
	return getch();
	#else
	struct termios oldt, newt;

	tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

	newt.c_lflag &= ~(ICANON | ECHO);

	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	const int result = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

	return result;
	#endif
}



void print_hand(std::vector<char>& hand) {
	std::cout << "{";
	for (size_t i = 0; i < hand.size(); ++i) {
		const char card = hand[i];
		std::cout << "'";
		if (card == '0') { std::cout << "10"; } else { std::cout << card; }
		std::cout << "'";

		if (i != hand.size() - 1) { std::cout << ", "; }
	}
	std::cout << "}\n";
}

void hit(std::vector<char>& public_deck, std::vector<char>& target_deck, const char& silent) {
	if (public_deck.empty()) {
		std::cout << "The public deck is empty!\n\n";
		return;
	}

	const char received_card = public_deck.back();
	target_deck.push_back(received_card);
	public_deck.pop_back();

	if (silent != '1') {
		std::cout << "Hit! Card received: ";
		if (received_card == '0') { std::cout << "10"; } else { std::cout << received_card; }
		std::cout << "\n\n";
	}
}

int get_value_from_hand(std::vector<char>& hand) {
	int value = 0;
	int ace_count = 0;
	for (char card : hand) {
		switch (card) {
			case '2':
				value += 2;
				break;
			case '3':
				value += 3;
				break;
			case '4':
				value += 4;
				break;
			case '5':
				value += 5;
				break;
			case '6':
				value += 6;
				break;
			case '7':
				value += 7;
				break;
			case '8':
				value += 8;
				break;
			case '9':
				value += 9;
				break;
			case '0':
				value += 10;
				break;
			case 'J':
				value += 10;
				break;
			case 'Q':
				value += 10;
				break;
			case 'K':
				value += 10;
				break;
			case 'A':
				ace_count++;
				break;
			default:
				break;
		}
	}

	for (int i = 0; i < ace_count; i++) { value += ((21-value < 11) ? 1 : 11); } // handle aces seperately
	return value;
}

std::tuple<std::vector<char>, std::vector<char>, std::vector<char>> init() {
	std::vector<char> public_deck = {'A','2','3','4','5','6','7','8','9','0','J','Q','K', 'A','2','3','4','5','6','7','8','9','0','J','Q','K', 'A','2','3','4','5','6','7','8','9','0','J','Q','K', 'A','2','3','4','5','6','7','8','9','0','J','Q','K'};
	std::vector<char> player_hand = {};
	std::vector<char> dealer_hand = {};

	shuffle(public_deck, 52);
	player_hand.reserve(11);
	dealer_hand.reserve(11);

	for (int i = 0; i < 2; i++) {
		player_hand.push_back(public_deck.back()); public_deck.pop_back();
		dealer_hand.push_back(public_deck.back()); public_deck.pop_back();
	}

	while ( (get_value_from_hand(dealer_hand) < 17) && (!public_deck.empty()) ) { hit(public_deck, dealer_hand, '1'); }
	std::cout << INTRO_MSG;
	return {public_deck, player_hand, dealer_hand};
}

void parse_action(std::vector<char>& public_deck, std::vector<char>& hand, const int& action, char& ready, char& fold) {
	switch (action) {
		case 1:
			hit(public_deck, hand, '0');
			break;
		case 2:
			ready = '1';
			break;
		case 3:
			fold = '1';
			break;
	}
}

void player_turn(std::vector<char>& public_deck, std::vector<char>& player_hand, char& player_stand, char& player_fold) {
	std::cout << PLAYER_TURN_MSG;
	print_hand(player_hand);
	const int action = get_char() - 48;
	if ( (action < 1) || (action > 3) ) {
		std::cout << "Invalid action, please choose again!\n\n";
		return player_turn(public_deck, player_hand, player_stand, player_fold);
	}

	parse_action(public_deck, player_hand, action, player_stand, player_fold);
}

short int check(std::vector<char>& player_hand, std::vector<char>& dealer_hand, char& stand, char& fold) {
	if ( get_value_from_hand(player_hand) > 21 ) {
		std::cout << "Player busted!\n";
		return 2;
	}

	if (stand == '1') {
		std::cout << "\nYour hand: "; print_hand(player_hand);
		std::cout << "Dealer's hand: "; print_hand(dealer_hand); std::cout << '\n';

		if ( get_value_from_hand(dealer_hand) > 21 ) {
			std::cout << "Dealer busted!\n";
			return 1;
		}

		if ( get_value_from_hand(player_hand) > get_value_from_hand(dealer_hand) ) {
			return 1;
		} else if ( get_value_from_hand(player_hand) == get_value_from_hand(dealer_hand) ) {
			return 3;
		} else {
			return 2;
		}
	} else if (fold == '1') {
		std::cout << "Player folded!\n";
		return 2;
	}

	return 0;
}

int main() {
	auto [public_deck, player_hand, dealer_hand] = init();
	char player_stand = '0';
	char player_fold = '0';

	short int should_end = 0;
	while (should_end == 0) {
		player_turn(public_deck, player_hand, player_stand, player_fold);

		switch ( check(player_hand, dealer_hand, player_stand, player_fold) ) {
			case 1:
				std::cout << "Player won!\n";
				should_end = 1;
				break;
			case 2:
				std::cout << "Dealer won!\n";
				should_end = 1;
				break;
			case 3:
				std::cout << "Push (nobody won)!\n";
				should_end = 1;
				break;
			default:
				break;
		}
	}

	std::cout << OUTRO_MSG;
	return 0;
}