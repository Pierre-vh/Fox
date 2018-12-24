add_source(
  unittests_src 

  "main.cpp"

  "Support/TestUtils.cpp"
  "Support/PrintObjects.cpp"

  "Fox/Lexer/LexerTests.cpp"
  "Fox/Common/UTF8Tests.cpp"
  "Fox/AST/ASTTests.cpp"
  "Fox/Common/SourceManagerTests.cpp"
  "Fox/Parser/LocParserTests.cpp"
  "Fox/Common/DiagnosticsTests.cpp"
  "Fox/Common/AllocatorsTests.cpp"
  "Fox/Common/DiagnosticVerifierTests.cpp"
  )
