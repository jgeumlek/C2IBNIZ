all:
	g++  -g -std=c++11 compiler.cpp parser.cpp walk_ast.cpp -o compiler
