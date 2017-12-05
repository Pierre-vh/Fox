Dumper = Visitor to dump the AST's data, used for debugging, but you can also call it freely to see the AST in text form.
Semantic = All semantics-related visitor : type checking, etc

Runtime Expression analyze:
https://www.reddit.com/r/cpp_questions/comments/7gfn1n/elegant_way_to_add_2_variants_depending_on_type/
* Make a generic arithmetic operation function (convert results to double, perform op, cast to result)
* Make a generic numeric comparison operation function
* Make a generic str comparison function
* Use double dispatch to dispatch result