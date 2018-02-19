#include <iostream>
using namespace std;
//#include <stdio.h>
bool testitest(){
	return false;
}
bool asfawgw = true;
bool x = testitest();
class Testitest{
	public:
		Testitest(){
			int x = 10;
			for(int i = 0 ; i < 10 ; i++){
				int j=9;
				x=20;
			}
			for(int i = 0 ; i < 10 ; i++){
				int j=9;
			}
			x=100;
			for(x=101;x>100;x--);
			bool a = testitest();
			a = testitest();
			bool b = true;
			b = testitest();
			bool c;
			a=true;
			c=true;
			c = testitest();
			testitest();
			testitest() && testitest();
			cout<<"public constructor"<<endl;
			initialize("abc");
			initialize(testitest()?"abc":"cde");
			if(testitest()&&testitest()||false)
				initialize(testitest()&&testitest()||testitest()||false);
		}
	private:
		void initialize(bool b){
		}
		void initialize(char* str){
			
			if(str[0]==123)
				return;
			cout<<length(str)<<endl;
		}
		bool huhu(){
			return testitest();
			int a;

		}
	protected:
		int length(char* str){
			int l = 0;
			char f;

			int i = 0;
			while(1){
				if(str[i]!=0){
					l++;
				}else{
					i++;
					break;
				}
			}
			f='a';
			return l;
		}
};

int main(int argc, const char **argv){
	Testitest *t = new Testitest();
	if(argc==2){
		if(argv[1][0] == '2'){
			int i = 2;
			i++;
			i = i-1;
			//for(int j=0;j<3;j++){
				2;
			//}
			3;
			//cout << 2 << endl;
		}else if(argv[1][0] == '3' && true)
			cout<<"test"<<endl;
	}

	if(1){
		2;
	}
}
