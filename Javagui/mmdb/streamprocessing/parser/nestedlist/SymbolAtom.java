package mmdb.streamprocessing.parser.nestedlist;


public class SymbolAtom extends Atom {

	private String content;

	public SymbolAtom(String content) {
		super();
		this.content = content;
	}

	public String getContent() {
		return content;
	}

	@Override
	public String printValueList() {
		return content.toString();
	}
	
	@Override
	public String printTypeList() {
		return "symbol";
	}

}
