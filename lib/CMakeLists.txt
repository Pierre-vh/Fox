add_subdirectory(Lexer)
add_subdirectory(Parser)
add_subdirectory(Common)
add_subdirectory(AST)
add_subdirectory(Driver)
add_subdirectory(Sema)

set(libfox_src
  ${lexer_src} 
  ${parser_src} 
  ${ast_src} 
  ${common_src}
  ${driver_src}
  ${sema_src}
  PARENT_SCOPE
)