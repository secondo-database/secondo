package mmdb.streamprocessing.parser.nestedlist;

public class BooleanAtom extends Atom {

	private Boolean content;

	public BooleanAtom(boolean b) {
		this.content = b;
	}

	public Boolean getContent() {
		return content;
	}

	@Override
	public String printValueList() {
		return content.toString();
	}
	
	@Override
	public String printTypeList() {
		return "bool";
	}

}
