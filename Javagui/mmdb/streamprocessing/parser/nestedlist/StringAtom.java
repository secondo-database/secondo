package mmdb.streamprocessing.parser.nestedlist;

public class StringAtom extends Atom {
	
	private String content;
	
	public StringAtom(String s) {
		this.content = s;
	}

	public String getContent() {
		return content;
	}

	@Override
	public String printValueList() {
		return "\"" + content.toString() + "\"";
	}
	
	@Override
	public String printTypeList() {
		return "string";
	}

}
