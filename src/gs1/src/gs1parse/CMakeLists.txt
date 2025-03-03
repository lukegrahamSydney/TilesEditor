add_library(
        gs1parse
        Diag.cpp              ../../include/gs1/parse/Diag.hpp
        Lexer.cpp             ../../include/gs1/parse/Lexer.hpp
        Parser.cpp            ../../include/gs1/parse/Parser.hpp
        Source.cpp            ../../include/gs1/parse/Source.hpp
        SyntaxTree.cpp        ../../include/gs1/parse/SyntaxTree.hpp
        SyntaxTreeVisitor.cpp ../../include/gs1/parse/SyntaxTreeVisitor.hpp
        Token.cpp             ../../include/gs1/parse/Token.hpp
)
