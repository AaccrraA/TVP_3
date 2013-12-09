//
// main.cpp
// PostMachine
//

#include <iostream>
#include <fstream>
#include <istream>
#include <string>
using namespace std;

struct Rule {
	string left;
	string right;
	Rule* next;
};

struct Expression {
	string name;
	string value;
	Expression* next;

	void initDefault() {
		name = "";
		value = "";
	}
};

struct PostMachine {
	Rule* R; // головной указатель на список правил
	string A;
	string X;
	string L;

	void initDefault() {
		R = NULL;
		A = "";
		X = "";
		L = "";
		loadFromFile();
	}

	void loadFromFile() {
		ifstream ifs("PM.txt");
		string field;
		if (ifs.fail()) {
			cout << "Ошибка открытия файла...";
			exit(0);
		}
		do {
			getline(ifs, field);
			if (field == "A:") {
				getline(ifs, A);
				for (int i = 0; i < A.length(); i++)
				if (A[i] == ' ' || A[i] == '\n')
					A.erase(i, 1);
				continue;
			}
			else if (field == "X:") {
				getline(ifs, X);
				for (int i = 0; i < X.length(); i++)
				if (X[i] == ' ' || X[i] == '\n')
					X.erase(i, 1);
				continue;
			}
			else if (field == "L:") {
				getline(ifs, L);
				for (int i = 0; i < L.length(); i++)
				if (L[i] == ' ' || L[i] == '\n')
					L.erase(i, 1);
				continue;
			}
			else if (field == "R:") {
				getline(ifs, field);
				if (R == NULL)
					R = new Rule;
				else {
					Rule* tRule = R;
					while (tRule->next != NULL)
						tRule = tRule->next;
					tRule = new Rule;
				}
				bool isLeft = true;
				for (int i = 0; i < field.length(); i++) {
					if (field[i] == ' ')
						continue;
					else if (field[i] == '>') {
						isLeft = false;
						continue;
					}
					else if (field[i] == '\n')
						break;
					if (isLeft)
						R->left.push_back(field[i]);
					else
						R->right.push_back(field[i]);
				}
				continue;
			}
			else
				break;
		} while (!ifs.eof());
	}

	// 0 - A, 1 - X
	int check(char c) {
		for (int i = 0; i < A.length(); i++)
		if (A[i] == c)
			return 0;
		for (int i = 0; i < X.length(); i++)
		if (X[i] == c)
			return 1;
		return -1;
	}

	// -1 - is not here
	int searchPos(int from, char c) {
		for (int i = from; i < L.length(); i++)
		if (c == L[i])
			return i;
		return -1;
	}

	bool defineValues(Expression* E1, Expression* E2) {
		bool isDefined = true;
		// xx/yy - xx/yy
		if (E1->name.length() == 2) {
			// xx -> x; yy -> y
			if (E1->name[0] == E1->name[1]) {
				E1->name = E1->name.erase(0);
				if (E1->value.length() % 2 == 0 && E1->value.length() > 0)
					E1->value = E1->value.substr(E1->value.length() / 2 - 1, E1->value.length() / 2);
				else
					isDefined = false;
			}
		}
		else if (E2->name.length() == 2) {
			if (E2->name[0] == E2->name[1]) {
				E2->name = E2->name.erase(0);
				if (E2->value.length() % 2 == 0 && E2->value.length() > 0)
					E2->value = E2->value.substr(E2->value.length() / 2 - 1, E2->value.length() / 2);
				else
					isDefined = false;
			}
		}

		// x/y - ??
		if (E1->name.length() == 1) {
			// x/y - x/y
			if (E1->name.length() == E2->name.length()) {
				if (E1->name == E2->name)
				if (E1->value != E2->value)
					isDefined = false;
			}
			// x/y - xy/yx
			else {
				// x/y - xy
				if (E2->name[0] == E1->name[0]) {
					E2->name.erase(0);
					E2->value = E2->value.substr(0, E1->value.length() - 1);
				}
				// x/y - yx
				else {
					E2->name.erase(1);
					E2->value = E2->value.substr(E2->value.length() - E1->value.length() - 1, E2->value.length() - 1);
				}
			}
		}
		// xy/yx - ??
		else {
			if (E2->name.length() == 2) {
				if (E1->name[0] == E2->name[1] && E1->name[1] == E2->name[0] && E1->value != E2->value)
					isDefined = false;
			}
			// xy - x
			else if (E1->name[0] == E2->name[0]) {
				E1->name.erase(0);
				E1->value = E1->value.substr(E2->value.length(), E2->value.length() - 1);
			}
			// yx - x
			else if (E1->value >= E2->value) {
				//substr(curPos, endPos - curPos);
				string S = E1->value.substr(E1->value.length() - E2->value.length(), E2->value.length());
				if (S == E2->value) {
					E1->name.erase(1);
					E1->value = E1->value.substr(0, E1->value.length() - E2->value.length());
				}
				else
					isDefined = false;
			}
			else
				isDefined = false;
		}
		return isDefined;
	}

	void startProcess() {
		bool isEnd = false;
		string lastL = "";
		while (!isEnd) {
			Rule* tmpR = R;

			while (tmpR != NULL) {
				int curPos = 0;
				Expression* E = NULL;
				Expression* tmpE = E;
				// Заполняем Expression
				for (int i = 0; i < R->left.length(); i++) {
					switch (check(R->left[i])) {
						// A
					case 0:
						if (i != 0 && check(R->left[i - 1]) == 1) {
							int endPos = searchPos(curPos, R->left[i]);
							if (endPos != -1) {
								tmpE->value = L.substr(curPos, endPos - curPos);
								curPos = endPos + 1;
							}
							else {
								cerr << "Программа завершена." << endl;
								exit(1);
							}
						}
						else {
							curPos = searchPos(curPos, R->left[i]);
						}
						break;
						// X
					case 1:
						if (i != 0 && check(R->left[i - 1]) == 1)
							tmpE->name += R->left[i];
						else {
							// Создание структуры
							if (E == NULL) {
								E = new Expression;
								tmpE = E;
								E->name += R->left[i];
							}
							else {
								tmpE->next = new Expression;
								tmpE->next->name += R->left[i];
								tmpE = tmpE->next;
								tmpE->next = NULL;
							}
						}
						break;
						// Error
					case -1:
						cerr << "Неизвестная ошибка." << endl;
						exit(-1);
						break;
					default:
						break;
					}
					if (curPos == -1) {
						cout << "Подходящего правила нет." << endl;
						exit(-1);
						break;
					}
				}
				
				tmpE = E;

				while (tmpE != NULL) {
					Expression* tmpE2 = E;
					while (tmpE2 != NULL) {
						if (!defineValues(tmpE, tmpE2)) {
							cout << "Программа завершена.";
							isEnd = true;
							return;
						}
						tmpE2 = tmpE2->next;
					}
					tmpE = tmpE->next;
				}

				cout << L << " -> ";

				//Будем искать в L строку lRuleStr и заменять ее на rRuleStr
				// x+x11= -> x+x=1
				// 11+1111= -> 11+11=1
				// 0111+1111=1 -> 11+11=11

				string lRuleStr = "", rRuleStr = "";

				//lRuleStr
				for (int i = 0; i < R->left.length(); i++) {
					tmpE = E;
					if (check(R->left[i]))
					while (tmpE != NULL) {
						if (tmpE->name[0] == R->left[i])
							lRuleStr += tmpE->value;
						tmpE = tmpE->next;
					}
					else
						lRuleStr += R->left[i];
				}
				//rRuleStr
				for (int i = 0; i < R->right.length(); i++) {
					tmpE = E;
					if (check(R->right[i]))
					while (tmpE != NULL) {
						if (tmpE->name[0] == R->right[i])
							rRuleStr += tmpE->value;
						tmpE = tmpE->next;
					}
					else
						rRuleStr += R->right[i];
				}
				bool isFound = false;
				int insertPos = 0;
				for (int i = 0; i < (L.length() - lRuleStr.length()); i++) {
					for (int j = 0; j < lRuleStr.length(); j++) {
						if (lRuleStr[j] != L[j]) {
							isFound = false;
							break;
						}
						else {
							if (!isFound)
								insertPos = i;
							isFound = true;
							continue;
						}
					}
					if (isFound)
						break;
				}

				L.replace(insertPos, lRuleStr.length(), rRuleStr);
				cout << L << endl;
				if (lastL == L) {
					cout << "Программа завершена т. к. рез-тат бесконечен." << endl;
					isEnd = true;
					break;
				}
				lastL = L;
			}
		}
	}
};

int main() {
	setlocale(LC_ALL, "rus");
	PostMachine PM;
	PM.initDefault();
	PM.startProcess();
	return 0;
}