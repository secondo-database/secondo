operator genTransaction alias GENTRANSACTION pattern op(_, _, _, _, _)
operator apriori alias APRIORI pattern _ op[_, _, _]
operator eclat alias ECLAT pattern _ op[_, _, _]
operator fpGrowth alias FPGROWTH pattern _ op[_, _, _]
operator createFpTree alias CREATEFPTREE pattern _ op[_, _]
operator mineFpTree alias MINEFPTREE pattern _ op[_, _]
operator genStrongRules alias GENRULES pattern _ op[_]
operator csvLoadTransactions alias CSVLOADTRANSACTION pattern op(_)
operator csvSaveTransactions alias CSVSAVETRANSACTION pattern _ op[_, _]
operator extendItemNames alias EXTENDITEMNAMES pattern _ op[_; funlist]
