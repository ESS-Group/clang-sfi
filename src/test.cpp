#include <iostream>
using namespace std;
//#include <stdio.h>
class Testitest{
	public:
		Testitest(){
			cout<<"public constructor"<<endl;
			initialize("abc");
		}
	private:
		void initialize(char* str){
			
			if(str[0]==123)
				return;
			cout<<length(str)<<endl;
		}
	protected:
		int length(char* str){
			int l = 0;
			//char c;
			int i = 0;
			while(1){
				if(str[i]!=0){
					l++;
				}else{
					i++;
					break;
				}
			}
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
		}else if(argv[1][0] == '3')
			cout<<"test"<<endl;
	}

	if(1){
		2;
	}
}
