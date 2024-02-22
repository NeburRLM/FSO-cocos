cocos0 : cocos0.c winsuport.o winsuport.h
	gcc -Wall cocos0.c winsuport.o -o cocos0 -lcurses


cocos1 : cocos1.c winsuport.o winsuport.h
	gcc -Wall cocos1.c winsuport.o -o cocos1 -lcurses -lpthread


cocos2 : cocos2.c winsuport.o winsuport.h
	gcc -Wall cocos2.c winsuport.o -o cocos2 -lcurses -lpthread
	

cocos3 : cocos3.c winsuport2.o memoria.o 
	gcc -Wall cocos3.c winsuport2.o memoria.o -o cocos3 -lcurses -lpthread


fantasma3 : fantasma3.c winsuport2.o winsuport2.h memoria.o
	gcc -Wall fantasma3.c winsuport2.o memoria.o -o fantasma3 -lcurses


cocos4 : cocos4.c winsuport2.o memoria.o 
	gcc -Wall cocos4.c winsuport2.o memoria.o semafor.o missatge.o -o cocos4 -lcurses -lpthread


fantasma4 : fantasma4.c winsuport2.o winsuport2.h memoria.o
	gcc -Wall fantasma4.c winsuport2.o memoria.o semafor.o missatge.o -o fantasma4 -lcurses -lpthread


winsuport2.o : winsuport2.c winsuport2.h
	gcc -Wall -c winsuport2.c -o winsuport2.o


memoria.o : memoria.c memoria.h
	gcc -c -Wall memoria.c -o memoria.o 


semafor.o : semafor.c semafor.h
	gcc -c -Wall semafor.c -o semafor.o 


missatge.o : missatge.c missatge.h
	gcc -c -Wall missatge.c -o missatge.o


clean:
	rm -f cocos0 cocos1 cocos2 cocos3 fantasma3 winsuport2.o memoria.o missatge.o
	
	
