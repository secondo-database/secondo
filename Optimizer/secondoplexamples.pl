secondo('open database ge', ResultList).

secondo_error_info(ErrorCode, ErrorString).

secondo('open database geo', ResultList);

secondo('query ten ', ResultList), print_term(ResultList).

% load pretty printing function for nested lists
[prettyprinter].

secondo([query, ten], X), pretty_print(X).
