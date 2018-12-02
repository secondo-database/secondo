

class BuildLoop extends React.Component {
    state = {
        manualfunction: "",
        queryfunction: "",
        functionchecked: false,
        criticalerror: false,
        errormsg: "",
        querytree: {id: "root", lhs:{}, rhs: {}}
    }
  
    handleChange = evt => {
        this.setState({ [evt.target.id]: evt.target.value });
    }

    setNativeValue = (element, value) => {
        let lastValue = element.value;
        element.value = value;
        let event = new Event("input", { target: element, bubbles: true });
        // React 15
        event.simulated = true;
        // React 16
        let tracker = element._valueTracker;
        if (tracker) {
            tracker.setValue(lastValue);
        }
        element.dispatchEvent(event);
    }

    handleBuild = (qf) => {
        const apicall = sspglobals.apiurl
            + "?cmd="      + "addquery"
            + "&hash="     + this.props.user.hash
            + "&function=" + qf

        // call the server to create this user
        fetch(apicall).then((response) => {
        if (response.ok)
            return response.json();
        else
            this.handleError("Error while using the API!", "danger");
            throw new Error("Error while using the API!");
        })
        .then((json) => {
            // returns an error
            if (json.result[0][1] == "success") {
                this.handleError("Query \n'"+qf+"'\n added!", "success")
                this.props.onLocationChange("list");
            } else {
                this.handleError("Failed to add Query: \n" + qf, "warning")
            }
        })
        .catch((err) => {
            this.handleError("Fetch catched an error!", "danger");
            console.log(err);
        });
    }
    
    iterateForFunction = (node) => {
        if (!Object.keys(node).length) return "";

        if (node.node == "attribute") {
            return "(attr t " + node.name + ")";
        }

        
        if (node.node == "connect") {
            if ((!Object.keys(node.lhs).length) || (!Object.keys(node.rhs).length)) {
                this.setState({criticalerror: true, errormsg: this.state.errormsg + "BREAKING: " + node.id + " is missing a child.\n"});
            }
            
            return ("(" 
                + node.type 
                + this.iterateForFunction(node.lhs) 
                + this.iterateForFunction(node.rhs) 
                + ")");
        }

        if (node.node == "compare") {
            const rhs = this.state[node.id+".rhs"];
            let saferhs = rhs || "";

            if (!Object.keys(node.lhs).length) {
                this.setState({criticalerror: true, errormsg: this.state.errormsg + "BREAKING: " + node.id + " is missing a child.\n"});
            } else {
                if (node.lhs.type == "string" || node.lhs.type == "text") {
                    const regex = /([^a-zA-Z0-9 ÄÜÖäüö@_.,:;+\-*/#~<>|°^!§€$%&()=?{\[\]}])/gm;
                    saferhs = saferhs.replace(regex, "");

                    if (saferhs != rhs) {
                        this.setState({errormsg: this.state.errormsg + "String ID '" + node.id + "' was adjusted. \n"});
                        this.setNativeValue(document.getElementById(node.id+".rhs"), saferhs);
                    }

                    saferhs = "'" + saferhs + "'";
                }

                if (node.lhs.type == "int") {
                    const regex = /([^\d-])/gm;
                    saferhs = saferhs.replace(regex, "");

                    if (saferhs != rhs) {
                        this.setState({errormsg: this.state.errormsg + "Integer ID '" + node.id + "' was adjusted. \n"});
                        this.setNativeValue(document.getElementById(node.id+".rhs"), saferhs);
                    }

                    if (saferhs.search("-") > 0) {
                        this.setState({criticalerror: true, errormsg: this.state.errormsg + "BREAKING: Integer ID '" + node.id + "' has a minus at an illegal position. \n"});
                    }

                    var count = 0;
                    for(var i=count=0; i<saferhs.length; count+=+("-"===saferhs[i++]));
                    if (count > 1) {
                        this.setState({criticalerror: true, errormsg: this.state.errormsg + "BREAKING: Integer ID '" + node.id + "' has more than one minus! \n"});
                    }
                }

                if (node.lhs.type == "real") {
                    const regex = /([^\d-.])/gm;
                    saferhs = saferhs.replace(regex, "");

                    if (saferhs != rhs) {
                        this.setState({errormsg: this.state.errormsg + "Real ID '" + node.id + "' was adjusted. \n"});
                        this.setNativeValue(document.getElementById(node.id+".rhs"), saferhs);
                    }

                    if (saferhs.search("-") > 0) {
                        this.setState({criticalerror: true, errormsg: this.state.errormsg + "BREAKING: Real ID '" + node.id + "' has a minus at an illegal position. \n"});
                    }

                    var count = 0;
                    for(var i=count=0; i<saferhs.length; count+=+("-"===saferhs[i++]));
                    if (count > 1) {
                        this.setState({criticalerror: true, errormsg: this.state.errormsg + "BREAKING: Real ID '" + node.id + "' has more than one minus! \n"});
                    }

                    for(var i=count=0; i<saferhs.length; count+=+("."===saferhs[i++]));
                    if (count > 1) {
                        this.setState({criticalerror: true, errormsg: this.state.errormsg + "BREAKING: Real ID '" + node.id + "' has more than one point! \n"});
                    }
                }

                if (node.lhs.type == "bool") {
                    const regex = /([^01])/gm;
                    saferhs = saferhs.replace(regex, "");

                    if (saferhs != rhs) {
                        this.setState({errormsg: this.state.errormsg + "Bool ID '" + node.id + "' was adjusted. \n"});
                        this.setNativeValue(document.getElementById(node.id+".rhs"), saferhs);
                    }

                    if (saferhs.length != 1) {
                        this.setState({criticalerror: true, errormsg: this.state.errormsg + "BREAKING: Bool ID '" + node.id + "' is no 0 or 1. \n"});
                    }
                }
            }
            
            return ("("
                + node.type
                + this.iterateForFunction(node.lhs) 
                + saferhs
                + ")");
        }

        return "error"
    }

    checkFunction = () => {
        // (fun(t(tuple((Kennzeichen string) (Ort string) (Vorwahl string) (BevT int))))(and(>(attr t BevT)10)(<(attr t BevT)100)))
        this.setState({criticalerror: false, errormsg: "", queryfunction: "", functionchecked: false});

        if ((!Object.keys(this.state.querytree.lhs).length) && (!Object.keys(this.state.querytree.rhs).length)) return;

        let fun =  "(fun(t(tuple";
            fun += this.props.user.tupledescr;
            fun += "))"
            fun += this.iterateForFunction(this.state.querytree);
            fun += ")";

        this.setState({queryfunction: fun, functionchecked: true});
    }

    renderCommand = (name, type) => {
        return (
            <span 
                key={name} 
                className="badge badge-info m-2"
                onDragStart = {(e) => this.onDragStart(e, {name: name, type: name, node: type})}
                draggable
            >{name}</span>
        )
    }

    renderExtract = (ex) => {
        ex.node = "attribute";

        return (
            <span
                className="badge badge-pill badge-secondary m-2"
                key={ex.name} 
                onDragStart = {(e) => this.onDragStart(e, ex)}
                draggable
            >{ex.name} {ex.type}</span>
        )
    }

    onDragStart = (ev, el) => {
        ev.dataTransfer.setData("el", JSON.stringify(el));
    }

    onDragOver = (ev) => {
        ev.preventDefault();
    }

    handleError = (reason, badge) => {
        this.props.onError(reason, badge);
    }

    onDrop = (e, target) => {
        e.stopPropagation();
        
        const el = JSON.parse(e.dataTransfer.getData("el"));

        if (target == "root") {
            if (el.node == "attribute") {
                this.handleError("attribute can't be dropped into root")
                return;
            }

            const querytree = {
                id: "root" + "." + el.node,
                node: el.node,
                type: el.type,
                name: el.name,
                lhs: {},
                rhs: {}
            }
            this.setState({...this.state, querytree, functionchecked: false, queryfunction: ""});

        } else {
            const parentId = target.substring(0, target.length-4);
            const targetSide = target.substring(target.length-3, target.length);
            const newLeaf = {
                id: target,
                node: el.node,
                type: el.type,
                name: el.name,
                lhs: {},
                rhs: {}
            }

            // recursive update of the querytree
            const querytree = this.updateTree(this.state.querytree, parentId, targetSide, newLeaf);

            this.setState({...this.state, querytree, functionchecked: false, queryfunction: ""});
        }
    }

    updateTree = (leaf, targetId, targetSide, newLeaf) => {
        if (!Object.keys(leaf).length) return {};

        if (leaf.id == targetId) {
            if (leaf.node == "connect" && newLeaf.node == "attribute") {
                this.handleError("can't drop an attribute into a connecting operator");
                return leaf;
            }

            if (leaf.node == "compare" && newLeaf.node == "connect") {
                this.handleError("can't drop a connect operator into a compare operator");
                return leaf;
            }

            if (leaf.node == "compare" && newLeaf.node == "compare") {
                this.handleError("can't drop a compare operator into a compare operator");
                return leaf;
            }

            if (targetSide == "lhs") {
                leaf.lhs = newLeaf;
            } else {
                leaf.rhs = newLeaf;
            }
        } else {
            leaf.rhs = this.updateTree(leaf.rhs, targetId, targetSide, newLeaf);
            leaf.lhs = this.updateTree(leaf.lhs, targetId, targetSide, newLeaf);
        }

        return leaf;
    }

    renderBuildComponents = (node, id) => {
        // empty object: return empty build-box
        if (!Object.keys(node).length) return (
            <div 
                key={id}
                className="build-box"
                onDragOver={(e)=>this.onDragOver(e)}
                onDrop={(e)=>this.onDrop(e, id)}
            ></div>
        );

        // render an connection node
        if (node.node == "connect") {
            return (
                <div 
                    className="build-box"
                    key={id}
                >
                    <h1>{node.type}</h1> <br/>
                    {this.renderBuildComponents(node.lhs, node.id+".lhs")}
                    {this.renderBuildComponents(node.rhs, node.id+".rhs")}
                </div>
            )
        };

        // render an comparison node
        if (node.node == "compare") {
            return (
                <div 
                    className="build-box"
                    key={id}
                >
                    {this.renderBuildComponents(node.lhs, node.id+".lhs")}
                    <h1>{node.type}</h1> <br/>
                    <input 
                        onChange={this.handleChange} 
                        type="text" 
                        className="form-control" 
                        id={node.id+".rhs"} 
                        placeholder="Enter a value" 
                    />
                </div>
            );
        }

        // render an attribute node
        if (node.node == "attribute")  {
            return (
                <div 
                    className="build-box"
                    key={id}
                >
                    <h1>{node.name}</h1> 
                    vom Typ 
                    <h4>{node.type}</h4>
                </div>
            );
        }
         
    }

    render() {
      return (
        <React.Fragment>
            <h1>Build</h1>
            <h3>Hello {this.props.user.email}</h3>
            <h4>{this.props.user.tupledescr}</h4>

            <hr className="hr-text" data-content="COMMANDS" />

            <h2 className="text-center">
                {this.renderCommand("and","connect")}
                {this.renderCommand("or", "connect")}
                {this.renderCommand("=",  "compare")}
                {this.renderCommand(">",  "compare")}
                {this.renderCommand("<",  "compare")}
                {this.renderCommand(">=", "compare")}
                {this.renderCommand("<=", "compare")}
            </h2>

            <hr className="hr-text" data-content="ATTRIBUTES" />

            <h2 className="text-center">
                {this.props.user.tupleextracts.map(te => this.renderExtract(te))}
            </h2>

            <hr className="hr-text" data-content="DROP HERE" />

            <div 
                style={{
                    minHeight: 200,
                    textAlign: "center",
                    borderStyle: "dashed",
                    borderWidth: 2,
                    backgroundColor: "#d0d0d0"
                }}
                onDragOver={(e)=>this.onDragOver(e)}
                onDrop={(e)=>this.onDrop(e, "root")}
            >
                {this.renderBuildComponents(this.state.querytree, "root")}
            </div>

            <StateOfFunction
                criticalerror={this.state.criticalerror}
                functionchecked={this.state.functionchecked}
                errormsg={this.state.errormsg}
                queryfunction={this.state.queryfunction}
            />
            
            <div className="clearfix">
                <button
                    onClick={() => this.handleBuild(this.state.queryfunction)}
                    className="btn btn-xl btn-primary m-2 float-right"
                    disabled={!this.state.functionchecked || this.state.criticalerror || this.state.queryfunction==""}
                >
                    Submit Query
                </button>
                <button
                    onClick={() => this.checkFunction()}
                    className="btn btn-xl btn-success m-2 float-right"
                >
                    Check Function
                </button>
                <button 
                    onClick={() => this.props.onLocationChange("list")}
                    className="btn btn-xl btn-warning m-2 float-right"
                >
                    Cancle
                </button>
            </div>
            
            <hr className="hr-text" data-content="DIRECT INPUT" />
            
            <div className="m-2 alert alert-warning" role="alert">
                Manually entering a query is not recommended and only here for testing purposes, since the function will not be checked againg and therefor can crash the SSP.
            </div>

            <input 
                onChange={this.handleChange} 
                type="text" 
                className="form-control m-2" 
                id="manualfunction" 
                placeholder="Enter a filter function... easy :)" 
            />

            <button 
                onClick={() => this.handleBuild(this.state.manualfunction)}
                className="btn btn-xl btn-danger m-2 float-right"
            >Add Manually Written Query</button>

        </React.Fragment>
      )
    }
  }
  
class StateOfFunction extends React.Component {
    getColorCode = () => {
        const {errormsg, queryfunction, criticalerror} = this.props;

        if (criticalerror) return "danger";
        if (!criticalerror && errormsg != "") return "warning";
        if (!criticalerror && queryfunction != "") return "success";
        return "info";
    }

    getMessage = () => {
        const {errormsg, queryfunction, criticalerror, functionchecked} = this.props;
        if (criticalerror) return errormsg;
        if (!functionchecked) return "no function yet... check function first";
        return ((errormsg == "") ? queryfunction : errormsg + " \n" + queryfunction);
    }

    render() {
        return (
            <div className={"m-2 alert text-center alert-"+this.getColorCode()} role="alert">
                {this.getMessage()}
            </div>
        )
    }
}