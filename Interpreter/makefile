all: a.out

a.out: a.out main.o Token.o Constants.o CommentRemover.o Tokenizer.o RDParser.o
	g++ -std=c++20 *.cpp

main.o: main.cpp Token.hpp Constants.hpp CommentRemover.hpp RDParser.hpp
	g++ -c main.cpp

Token.o: Token.cpp Token.hpp
	g++ -c Token.cpp

Constants.o: Constants.hpp
	g++ -c Constants.hpp

CommentRemover.o: CommentRemover.cpp CommentRemover.hpp
	g++ -c CommentRemover.cpp

Tokenizer.o: Tokenizer.cpp Tokenizer.hpp
	g++ -c Tokenizer.cpp

RDParser.o: RDParser.cpp RDParser.hpp
	g++ -c RDParser.cpp

clean:
	rm -rf *.o
