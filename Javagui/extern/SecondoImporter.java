package extern;
import sj.lang.ListExpr;

public interface SecondoImporter{
  ListExpr getList(String FileName);
  String getErrorString();
}