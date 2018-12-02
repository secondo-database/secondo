

class BuildJoin extends React.Component {
    state = {
        manualfunction: "",
        queryfunction: "",
        functionchecked: false,
        criticalerror: false,
        errormsg: "",
        querytree: [
            {tupleextract: null, comparetype: null, comparevalue: null, neighbor: null},
            {tupleextract: null, comparetype: null, comparevalue: null, neighbor: null},
            {tupleextract: null, comparetype: null, comparevalue: null, neighbor: null}]
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
    
    iterateForFunction = (element, index) => {
        if (element.tupleextract == null) {
            this.setState((prev) => ({criticalerror: true, errormsg: prev.errormsg + "BREAKING: @" + index + " it's unchoosen what to test. \n"}));
            return "breaking error";
        }
        
        if (element.comparetype == null) {
            this.setState((prev) => ({criticalerror: true, errormsg: prev.errormsg + "BREAKING: @" + index + " it's unchoosen how to test. \n"}));
            return "breaking error";
        }

        // if (element.comparevalue == null) {
        //     this.setState((prev) => ({criticalerror: true, errormsg: prev.errormsg + "BREAKING: @" + index + " no value was given. \n"}));
        //     return "breaking error";
        // }

        const extract = this.props.user.tupleextracts[element.tupleextract];
        const chain = (element.neighbor == null) ? "" : " ";

        if (extract.type == "string" || extract.type == "text") {
            if (element.comparevalue == null) element.comparevalue = "";

            const regex = /([^a-zA-Z0-9 ÄÜÖäüö@_.,:;+\-*/#~<>|°^!§€$%&()=?{\[\]}])/gm;
            const value = element.comparevalue.replace(regex, "");

            if (value != element.comparevalue) {
                this.setState((prev) => ({errormsg: prev.errormsg + "String @" + index + " was adjusted. \n"}));
                this.setNativeValue(document.getElementById("valuebox-"+index), value);
            }

            return "(" + extract.name + "_" + element.comparetype + " \"" + value + "\")" + chain;
        }

        if (extract.type == "int") {
            if (element.comparevalue == null) element.comparevalue = 0

            const value = parseInt(element.comparevalue);

            if (isNaN(value)) {
                this.setState((prev) => ({criticalerror: true, errormsg: prev.errormsg + "BREAKING: Integer @" + index + " could not be parsed. \n"}));
                return "error";
            }

            return "(" + extract.name + "_" + element.comparetype + " " + value + ")" + chain;
        }

        if (extract.type == "real") {
            if (element.comparevalue == null) element.comparevalue = 0.0;

            const value = parseFloat(element.comparevalue);

            if (isNaN(value)) {
                this.setState((prev) => ({criticalerror: true, errormsg: prev.errormsg + "BREAKING: Real @" + index + " could not be parsed. \n"}));
                return "error";
            }

            return "(" + extract.name + "_" + element.comparetype + " " + value + ")" + chain;
        }

        if (extract.type == "bool") {
            const value = (element.comparevalue == "TRUE") ? "TRUE" : "FALSE";
            return "(" + extract.name + "_" + element.comparetype + " " + value + ")" + chain;
        }

        return "error";
    }

    checkFunction = () => {
        this.setState({criticalerror: false, errormsg: "", queryfunction: "", functionchecked: false});

        let index = 0;

        let fun =  "(";

            do {
                fun += this.iterateForFunction(this.state.querytree[index], index)
                index++;
                console.log(this.state.querytree[index-1].neighbor);
            } while (this.state.querytree[index-1].neighbor != null);

            fun += ")";

        this.setState({queryfunction: fun, functionchecked: true});
    }

    handleError = (reason, badge) => {
        this.props.onError(reason, badge);
    }

    handleBCSelectExtractChange = (bcId, extractId) => {
        let qt = this.state.querytree;
        qt[bcId].tupleextract = (isNaN(parseInt(extractId)) ? null : parseInt(extractId));
        qt[bcId].comparetype = null;
        qt[bcId].comparevalue = null;

        // this.setNativeValue(document.getElementById("valuebox-"+bcId), "");

        this.setState({querytree: qt});
    }

    handleBCSelectComparatorChange = (bcId, comparator) => {
        let qt = this.state.querytree;
        qt[bcId].comparetype = comparator;
        // qt[bcId].comparevalue = null;
        this.setState({querytree: qt});
    }

    handleBCCompareValueChange = (bcId, value) => {
        let qt = this.state.querytree;
        qt[bcId].comparevalue = value;
        this.setState({querytree: qt});
        if (this.props.user.tupleextracts[qt[bcId].tupleextract].type != "bool") {
            this.setNativeValue(document.getElementById("valuebox-"+bcId), value);
        } else {
            this.setNativeValue(document.getElementById("valuebox-"+bcId), "");
        }
    }

    handleAddAction = (bcId, action) => {
        let qt = this.state.querytree;
        if (action == "+") {
            qt[bcId].neighbor = "and";
            qt[bcId+1] = {tupleextract: null, comparetype: null, comparevalue: null, neighbor: null};
        } else

        if (action == "-") {
            qt[bcId-1].neighbor = null;
            qt[bcId] = {tupleextract: null, comparetype: null, comparevalue: null, neighbor: null};
        } else 

        if (action == "and") {
            qt[bcId].neighbor = "and";
        } else 

        if (action == "or") {
            qt[bcId].neighbor = "or";
        } else {
            return;
        }

        this.setState({querytree: qt});
    }

    render() {
      return (
        <React.Fragment>
            <h1>Build</h1>
            <h3>Hello {this.props.user.email}</h3>
            <h4>{this.props.user.tupledescr}</h4>

            <hr className="hr-text" data-content="BUILD HERE" />

            <div 
                style={{
                    minHeight: 200,
                    textAlign: "center",
                    borderStyle: "dashed",
                    borderWidth: 2,
                    backgroundColor: "#d0d0d0",
                    display: "flex"
                }}
                onDragOver={(e)=>this.onDragOver(e)}
                onDrop={(e)=>this.onDrop(e, "root")}
            >
                <BuildComponent 
                    key={0} 
                    num={0}
                    extracts={this.props.user.tupleextracts} 
                    leaf={this.state.querytree[0]} 
                    handleBCSelectExtractChange={this.handleBCSelectExtractChange}
                    handleBCSelectComparatorChange={this.handleBCSelectComparatorChange}
                    handleBCCompareValueChange={this.handleBCCompareValueChange}
                    handleAddAction={this.handleAddAction}
                />

                {this.state.querytree[0].neighbor == null ? null : <BuildComponent 
                    key={1} 
                    num={1}
                    extracts={this.props.user.tupleextracts} 
                    leaf={this.state.querytree[1]} 
                    handleBCSelectExtractChange={this.handleBCSelectExtractChange}
                    handleBCSelectComparatorChange={this.handleBCSelectComparatorChange}
                    handleBCCompareValueChange={this.handleBCCompareValueChange}
                    handleAddAction={this.handleAddAction}
                />}

                {(this.state.querytree[0].neighbor == null || this.state.querytree[1].neighbor == null) ? null : <BuildComponent 
                    key={2} 
                    num={2}
                    extracts={this.props.user.tupleextracts} 
                    leaf={this.state.querytree[2]} 
                    handleBCSelectExtractChange={this.handleBCSelectExtractChange}
                    handleBCSelectComparatorChange={this.handleBCSelectComparatorChange}
                    handleBCCompareValueChange={this.handleBCCompareValueChange}
                    handleAddAction={this.handleAddAction}
                />}
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

class BuildComponent extends React.Component {
    getExtracts = () => {
        const {extracts} = this.props;
        let options = [];

        extracts.forEach((element, index) => {
            options.push(<option key={index} value={index}>{element.name} ({element.type})</option>)
        });
        
        return options;
    }

    getExtract = () => {
        const {leaf, extracts} = this.props;
        if (leaf.tupleextract == null) return {name: null, type: null};

        return extracts[leaf.tupleextract];
    }

    getComparator = () => {
        const type = this.getExtract().type;
        if (type == "string" || type == "bool") {
            return <h1>=</h1>;
        }

        if (type == "int" || type == "real") return <select 
            className="form-control" 
            onChange={this.changeComparator}
            value={this.props.leaf.comparetype}>
                <option key="-1" value={-1}>Choose comparator.</option>
                <option key="eq" value="eq"> = </option>
                <option key="gt" value="gt"> &gt; </option>
                <option key="lt" value="lt"> &lt; </option>
            </select>;
        
        return null;
    }

    getValuebox  = () => {
        const type = this.getExtract().type;

        let isChecked = (this.props.leaf.comparevalue == "TRUE") ? true : false;
        
        if (type == "string") return <input type="text"            key={"valuebox-"+this.props.num} id={"valuebox-"+this.props.num} onChange={this.changeValue} defaultValue={this.props.leaf.comparevalue} value={this.props.leaf.comparevalue} />;
        if (type == "int")    return <input type="number" step="1" key={"valuebox-"+this.props.num} id={"valuebox-"+this.props.num} onChange={this.changeValue} defaultValue={this.props.leaf.comparevalue} value={this.props.leaf.comparevalue} />;
        if (type == "real")   return <input type="text" step="any" key={"valuebox-"+this.props.num} id={"valuebox-"+this.props.num} onChange={this.changeValue} defaultValue={this.props.leaf.comparevalue} value={this.props.leaf.comparevalue} />;
        if (type == "bool")   return <input type="checkbox"        key={"valuebox-"+this.props.num} id={"valuebox-"+this.props.num} onChange={this.changeValue} defaultChecked={isChecked} checked={isChecked}/>;

        return null;
    }

    changeExtract = (event) => {
        let val;
        if (event.target.value == -1) {
            val=null;
        } else {
            val=event.target.value;
        }

        this.props.handleBCSelectExtractChange(this.props.num, val);

        const type = this.getExtract().type;
        if (type == "string" || type == "bool") {
            this.changeComparator(null);
        }
    }

    changeComparator = (event) => {
        if (event == null) {
            this.props.handleBCSelectComparatorChange(this.props.num, "eq");
            return;
        }

        let val;
        if (event.target.value == -1) {
            val=null;
        } else {
            val=event.target.value;
        }

        this.props.handleBCSelectComparatorChange(this.props.num, val);
    }

    changeValue = (event) => {
        let val = event.target.value;

        if (event.target.type == "checkbox") {
            val = event.target.checked ? "TRUE" : "FALSE";
        }

        this.props.handleBCCompareValueChange(this.props.num, val);
    }

    handleAddEvent = (event) => {
        let action = event.target.innerHTML;
        if (action.length > 1) action = event.target.value;
        
        this.props.handleAddAction(this.props.num, action);
    }

    getAddButtons = () => {
        const {leaf, num} = this.props;
        let btns = [];

        if (leaf.neighbor == null && num<2) btns.push(<button onClick={this.handleAddEvent} className="btn bnt-sm btn-primary add-button">+</button>);
        if (leaf.neighbor == null && num>0) btns.push(<button onClick={this.handleAddEvent} className="btn bnt-sm btn-warning add-button">-</button>);
        if (leaf.neighbor != null) btns.push(<span className="badge badge-primary">and</span>);
        // if (leaf.neighbor != null) btns.push(<select 
        //     className="form-control" 
        //     onChange={this.handleAddEvent}
        //     defaultValue={leaf.neighbor}
        //     >
        //         <option key="and" value="and"> AND </option>
        //         <option key="or" value="or"> OR </option>
        //     </select>);
        
        return <div className="add-buttons">{btns}</div>;
    }


    render() {
        return (<React.Fragment>
            <div className="build-box">
                <select className="form-control" value={this.props.leaf.tupleextract} id={"selectExtract-"+this.props.num} onChange={this.changeExtract}>
                    <option key="-1" value={-1}>What should be tested?</option>
                    {this.getExtracts()}
                </select> 
                {this.getComparator()} 
                {this.getValuebox()}
            </div>
            {this.getAddButtons()}
        </React.Fragment>)
    }
}