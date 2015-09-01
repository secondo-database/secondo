package mmdb.streamprocessing.parser.nestedlist;

public class IntegerAtom extends Atom {

	private Integer content;

	public IntegerAtom(int i) {
		this.content = i;
	}

	public Integer getContent() {
		return content;
	}

	@Override
	public String printValueList() {
		return content.toString();
	}

	@Override
	public String printTypeList() {
		return "int";
	}
	
}
