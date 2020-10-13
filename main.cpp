#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <string.h>
using namespace std;

// all instructions
const char insts[5][5]{ "ADD", "SUB", "MUL", "DIV", "ADDI" };
const char ops[5]{ '+','-','*','/','+' };
const int inst_num = 5;

struct ROB {
	int index;
	int reg_des;
	int val;
	bool done;
};

struct RS {
	int index;
	int op;
	bool done[2];
	ROB* rob_ptr[2];
	int val[2];
	int cyc;
	RS() :index(-1), op(-1), cyc(-1) {};
};

struct INST {
	int op;
	int reg_des;
	int reg[2];
};

int insts_cyc[5], reg_num, add_rs_num, mul_rs_num, total_rs_num;
vector<int> regs;
vector<RS> rs;
RS add_exe, mul_exe;
vector<ROB*> rats;
vector<INST> inst_vec;
ROB* rob;

INST dist(char inst[20])
{
	INST temp;
	char dis_inst[5];
	int i, j;
	for (i = 0; inst[i] != ' '; i++)
		dis_inst[i] = inst[i];
	dis_inst[i] = '\0';
	for (j = 0; j < inst_num; j++)
		if (strcmp(insts[j], dis_inst) == 0)
			break;
	temp.op = j;
	for (; inst[i] != 'F'; i++);
	i++;
	temp.reg_des = (int)atoi(inst + i);
	for (; inst[i] != 'F'; i++);
	i++;
	temp.reg[0] = (int)atoi(inst + i);
	if (j == 4)
		for (; inst[i] != ' '; i++);
	else
		for (; inst[i] != 'F'; i++);
	i++;
	temp.reg[1] = (int)atoi(inst + i);
	return temp;
}

void inputHandler()
{
	char inst[20];
	cout << "Instruction Cycle" << endl;
	for (int i = 0; i < inst_num; i++) {
		cout << insts[i] << " : ";
		cin >> insts_cyc[i];
	}
	cout << "Number of Register : ";
	cin >> reg_num;
	regs.assign(reg_num, 0);
	rats.assign(reg_num, nullptr);
	cout << "Value of Register" << endl;
	for (int i = 0; i < reg_num; i++) {
		cout << "F" << i + 1 << " : ";
		cin >> regs[i];
	}
	cout << "Number of Reservation Station" << endl;
	cout << "add/sub/addi : ";
	cin >> add_rs_num;
	cout << "mul/div : ";
	cin >> mul_rs_num;
	total_rs_num = add_rs_num + mul_rs_num;
	rs.assign(total_rs_num, RS());
	cout << "Instruction : " << endl;
	cin.getline(inst, 20);
	while (1) {
		cin.getline(inst, 20);
		if (!strlen(inst))
			break;
		inst_vec.push_back(dist(inst));
	}
	rob = new ROB[inst_vec.size()]{};
	return;
}

bool issue(int index)
{
	int i;
	INST inst = inst_vec[index];
	if (inst.op < 2 || inst.op == 4) {
		for (i = 0; i < add_rs_num; i++) {
			if (rs[i].op == -1)
				break;
			else if (rs[i].op != -1 && i == add_rs_num - 1)
				return false;
		}
	}
	else {
		for (i = add_rs_num; i < total_rs_num; i++) {
			if (rs[i].op == -1)
				break;
			else if (rs[i].op != -1 && i == total_rs_num - 1)
				return false;
		}
	}
	rs[i].op = inst.op;
	rs[i].index = index;
	if (rats[inst.reg[0] - 1] == nullptr) {
		rs[i].done[0] = true;
		rs[i].val[0] = regs[inst.reg[0] - 1];
	}
	else {
		rs[i].done[0] = false;
		rs[i].rob_ptr[0] = rats[inst.reg[0] - 1];
	}
	if (inst.op == 4) {
		rs[i].done[1] = true;
		rs[i].val[1] = inst.reg[1];
	}
	else {
		if (rats[inst.reg[1] - 1] == nullptr) {
			rs[i].done[1] = true;
			rs[i].val[1] = regs[inst.reg[1] - 1];
		}
		else {
			rs[i].done[1] = false;
			rs[i].rob_ptr[1] = rats[inst.reg[1] - 1];
		}
	}
	ROB tem;
	tem.done = false;
	tem.reg_des = inst.reg_des;
	tem.index = index;
	rob[index] = tem;
	rats[inst.reg_des - 1] = rob + index;
	return true;
}

int exe(int cyc)
{
	int  stat = 0;
	if (add_exe.cyc == cyc) {
		stat = 1;
		switch (add_exe.op) {
		case 0:
			rob[add_exe.index].val = add_exe.val[0] + add_exe.val[1];
			rob[add_exe.index].done = true;
			break;
		case 1:
			rob[add_exe.index].val = add_exe.val[0] - add_exe.val[1];
			rob[add_exe.index].done = true;
			break;
		case 4:
			rob[add_exe.index].val = add_exe.val[0] + add_exe.val[1];
			rob[add_exe.index].done = true;
			break;
		}
		add_exe.op = -1;
	}
	if (mul_exe.cyc == cyc) {
		stat = 1;
		switch (mul_exe.op) {
		case 2:
			rob[mul_exe.index].val = mul_exe.val[0] * mul_exe.val[1];
			rob[mul_exe.index].done = true;
			break;
		case 3:
			if (mul_exe.val[1] == 0) {
				stat = -1;
				break;
			}
			rob[mul_exe.index].val = mul_exe.val[0] / mul_exe.val[1];
			rob[mul_exe.index].done = true;
			break;
		}
		mul_exe.op = -1;
	}
	return stat;
}

void broadcast()
{
	for (int i = 0; i < total_rs_num; i++) {
		for (int j = 0; j < 2; j++) {
			if (!rs[i].done[j]) {
				if (rs[i].rob_ptr[j]->done) {
					rs[i].done[j] = true;
					rs[i].val[j] = rs[i].rob_ptr[j]->val;
				}
			}
		}
	}
	for (int i = 0; i < reg_num; i++) {
		if (rats[i] != nullptr && rats[i]->done) {
			regs[rats[i]->reg_des - 1] = rats[i]->val;
			rats[i] = nullptr;
			break;
		}
	}
}

void dispatch(int cyc)
{
	if (add_exe.op == -1) {
		for (int i = 0; i < add_rs_num; i++) {
			if (rs[i].done[0] == true && rs[i].done[1] == true) {
				add_exe = rs[i];
				add_exe.cyc = cyc + insts_cyc[add_exe.op];
				rs[i] = RS();
				break;
			}
		}
	}
	if (mul_exe.op == -1) {
		for (int i = add_rs_num; i < total_rs_num; i++) {
			if (rs[i].done[0] == true && rs[i].done[1] == true) {
				mul_exe = rs[i];
				mul_exe.cyc = cyc + insts_cyc[mul_exe.op];
				rs[i] = RS();
				break;
			}
		}
	}
}

int main()
{
	inputHandler();
	int inst = 0, cyc = 0;
	int stat;
	bool finish;
	while (1) {
		cyc++;
		stat = exe(cyc);
		if (stat == -1) {
			cout << "div zero" << endl;
			break;
		}
		else if (stat == 1) {
			broadcast();
		}
		dispatch(cyc);
		if (inst < inst_vec.size() && issue(inst))
			inst++;
		cout << "Cycle:" << cyc << endl << endl;
		cout << "RF" << endl;
		cout << "---------------" << endl;
		for (int i = 0; i < reg_num; i++) {
			cout << "F" << i + 1 << " " << regs[i] << endl;
		}
		cout << "---------------" << endl << endl << endl;
		cout << "RAT" << endl;
		cout << "---------------" << endl;
		for (int i = 0; i < reg_num; i++) {
			cout << "F" << i + 1 << " ";
			if (rats[i] != nullptr)
				cout << "ROB" << rats[i]->index + 1;
			cout << endl;
		}
		cout << "---------------" << endl << endl << endl;
		cout << "ROB" << endl;
		cout << "---------------" << endl;
		finish = true;
		for (int i = 0; i < inst_vec.size(); i++) {
			if (!rob[i].done) {
				finish = false;
				cout << "ROB" << i + 1 << " ";
				if (rob[i].reg_des)
					cout << "F" << rob[i].reg_des;
				cout << endl;
			}
		}
		cout << "---------------" << endl << endl << endl;
		cout << "RS" << endl;
		cout << "---------------" << endl;
		for (int i = 0; i < add_rs_num; i++) {
			cout << "RS" << i + 1 << " " << ops[rs[i].op] << " ";
			for (int j = 0; j < 2; j++) {
				if (rs[i].op != -1) {
					if (rs[i].done[j])
						cout << rs[i].val[j] << " ";
					else
						cout << "ROB" << rs[i].rob_ptr[j]->index+1 << " ";
				}
			}
			cout << endl;
		}
		cout << "---------------" << endl << endl;
		cout << "---------------" << endl;
		for (int i = add_rs_num; i < total_rs_num; i++) {
			cout << "RS" << i + 1 << " " << ops[rs[i].op] << " ";
			for (int j = 0; j < 2; j++) {
				if (rs[i].op != -1) {
					if (rs[i].done[j])
						cout << rs[i].val[j] << " ";
					else
						cout << "ROB" << rs[i].rob_ptr[j]->index+1 << " ";
				}
			}
			cout << endl;
		}
		cout << "---------------" << endl << endl;
		if (finish)
			break;
	}
}