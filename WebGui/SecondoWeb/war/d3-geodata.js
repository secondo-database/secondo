
//Width and height of the svg
var w ;
var h ;
var padding = 45;
var svg ;
var xAxis ;
var yAxis ;

var datasetPoints = [];
var datasetLines = [];

function resizeView(width, height){
	//Create SVG element
	/*svg = d3.select(div)
				.append("svg")
				.attr("id", "mainsvg")
				.attr("width", w)
				.attr("height", h);*/
	w = height;
	h = height;
	
}


function showPointArray(div, width, height){
	
	w = height;
	h = height;
	
	//Create scale functions for x and y axis
	var xScale = d3.scale.linear()
                         .domain([d3.min(datasetPoints, function(d) { return d[0]; }), d3.max(datasetPoints, function(d) { return d[0]; })])//get minimum and maximum of the first entry of the pointarray
                         .range([padding, w - padding]); //w, the SVGÕs width. without padding 0,w
	
	var yScale = d3.scale.linear()
                         .domain([d3.min(datasetPoints, function(d) { return d[1]; }), d3.max(datasetPoints, function(d) { return d[1]; })])
                         .range([h - padding, padding]); //without padding h,0
	
	//scales the radius of the circle
	var rScale = d3.scale.linear()
                         .domain([d3.min(datasetPoints, function(d) { return d[1]; }), d3.max(datasetPoints, function(d) { return d[1]; })])
                         .range([2, 5]); // radius values will always fall within the range of 2,5
	
	var formatTick = d3.format(".1");
	//var commasFormatter = d3.format(",.0f");//will display integers with comma-grouping for thousands
	
	//get the max number of values for the axis
	var axisXScale;
	var axisYScale;
	   if(d3.max(datasetPoints, function(d) { return d[0]; }) > d3.max(datasetPoints, function(d) { return d[1]; })){
		   axisXScale = xScale;
		   axisYScale = d3.scale.linear()
                                .domain([d3.min(datasetPoints, function(d) { return d[0]; }), d3.max(datasetPoints, function(d) { return d[0]; })])
                                .range([h - padding, padding]);
	   }
	   else{
		   axisXScale = d3.scale.linear()
                                .domain([d3.min(datasetPoints, function(d) { return d[1]; }), d3.max(datasetPoints, function(d) { return d[1]; })])
                                .range([padding, w - padding]);
		   axisYScale = yScale;
	   }
	
	//Define X axis
	xAxis = d3.svg.axis()
					  .scale(axisXScale)
					  .orient("bottom")
					  .ticks(5);  //Set rough # of ticks;
	                  //.tickFormat(commasFormatter);
	
	//Define Y axis
	yAxis = d3.svg.axis()
	                  .scale(axisYScale)
	                  .orient("left")
	                  .ticks(5);
	                  //.tickFormat(formatTick);
		
	//Create SVG element
	svg = d3.select(div)
				.append("svg")
				.attr("id", "mainsvg")
				.attr("width", w)
				.attr("height", h);

	//Create circles
	svg.selectAll("circle")
	      .data(datasetPoints)
	      .enter()
	      .append("circle")
	      .attr("cx", function(d) {
	   		return axisXScale(d[0]);
	       })
	      .attr("cy", function(d) {
	   		return axisYScale(d[1]);
	      })
	      .attr("r", function(d) {
            return rScale(d[1]);
          })
          .attr("fill", "red");

	//Create labels
	/*svg.selectAll("text")
	      .data(datasetPoints)
	      .enter()
	      .append("text")
	      .text(function(d) {
	   		return d[0] + "," + d[1];
	      })
	      .attr("x", function(d) {
	   		return xScale(d[0]);
	      })
	      .attr("y", function(d) {
	   		return yScale(d[1]);
	      })
	      .attr("font-family", "sans-serif")
	      .attr("font-size", "11px")
	      .attr("fill", "red");*/
	
	//Create X axis
	svg.append("g")
		.attr("class", "axis")
		.attr("transform", "translate(0," + (h - padding) + ")")// transform the entire axis group (g), pushing it to the bottom:
		.call(xAxis);
	
	//Create Y axis
	svg.append("g")
	    .attr("class", "axis")
	    .attr("transform", "translate(" + padding + ",0)")
	    .call(yAxis);
}

function showLineArray(div, width, height){
	
	w = height;
	h = height;
	
	//check which scale is the biggest one and take this one for the axis and range of points
	//get the max and min number of values for the axis	
	var xScale;
	var yScale;
	
	var maximumArray = [d3.max(datasetLines, function(d) { return d[0]; }), d3.max(datasetLines, function(d) { return d[1]; }), 
			            d3.max(datasetLines, function(d) { return d[2]; }), d3.max(datasetLines, function(d) { return d[3]; })];
	
	var minimumArray = [d3.min(datasetLines, function(d) { return d[0]; }), d3.min(datasetLines, function(d) { return d[1]; }), 
			            d3.min(datasetLines, function(d) { return d[2]; }), d3.min(datasetLines, function(d) { return d[3]; })];
			             
    var maximum = d3.max(maximumArray);  
    var minimum = d3.min(minimumArray);

    var indexMaximum = maximumArray.indexOf(maximum); 
    var indexMinimum = minimumArray.indexOf(minimum); 
    
        xScale = d3.scale.linear()
                         .domain([d3.min(datasetLines, function(d) { return d[indexMinimum]; }), d3.max(datasetLines, function(d) { return d[indexMaximum]; })])
                         .range([padding, w - padding]);
    	yScale = d3.scale.linear()
                         .domain([d3.min(datasetLines, function(d) { return d[indexMinimum]; }), d3.max(datasetLines, function(d) { return d[indexMaximum]; })])
                         .range([h - padding, padding]);
		
	//var formatTick = d3.format(".1");
    //var commasFormatter = d3.format(",.0f");//will display integers with comma-grouping for thousands
	
	//Define X axis
	xAxis = d3.svg.axis()
					  .scale(xScale)
					  .orient("bottom")
					  .ticks(5);  //Set rough # of ticks;
	                  //.tickFormat(formatTick);
	
	//Define Y axis
	yAxis = d3.svg.axis()
	                  .scale(yScale)
	                  .orient("left")
	                  .ticks(5);
	                  //.tickFormat(formatTick);
		
	//Create SVG element
	svg = d3.select(div)
				.append("svg")
				.attr("id", "mainsvg")
				.attr("width", w)
				.attr("height", h);

	//Create lines
	svg.selectAll("line")
	      .data(datasetLines)
	      .enter()
	      .append("line")
	      .attr("x1", function(d) {
	   		return xScale(d[0]);
	       })
	      .attr("y1", function(d) {
	   		return yScale(d[1]);
	      })
	      .attr("x2", function(d) {
	   		return xScale(d[2]);
	       })
	      .attr("y2", function(d) {
	   		return yScale(d[3]);
	      })
         .attr("stroke-width", 2)
         .attr("stroke", "black");
	
	
	//Create X axis
	svg.append("g")
		.attr("class", "axis")
		.attr("shape-rendering", "crispEdges")
		.attr("transform", "translate(0," + (h - padding) + ")")
		.call(xAxis);
	
	//Create Y axis
	svg.append("g")
	    .attr("class", "axis")
	    .attr("shape-rendering", "crispEdges")
	    .attr("transform", "translate(" + padding + ",0)")
	    .call(yAxis);
}

//Adds a point to the dataset
function addPointToDataset(x, y){
	var newNumber1 = x;	
	var newNumber2 = y;	
	datasetPoints.push([newNumber1, newNumber2]);	//Add new number to array
}

//Adds a line to the dataset
function addLineToDataset(x1, y1, x2, y2){
	var newNumber1 = x1;	
	var newNumber2 = y1;	
	var newNumber3 = x2;
	var newNumber4 = y2;
	datasetLines.push([newNumber1, newNumber2, newNumber3, newNumber4]);	//Add new numbers to array
}


//Deletes all points in the array by removing references to them
function deleteAllPoints() {
	if (datasetPoints) {
	    for (i in datasetPoints) {
	      delete i;
	    }
    datasetPoints.length = 0;
  }
}

//Deletes all points in the array by removing references to them
function deleteAllLines() {
	if (datasetLines) {
	    for (i in datasetLines) {
	      delete i;
	    }
    datasetLines.length = 0;
  }
}

//removes the svg from the view
function removeSVG(){
	d3.select("svg").remove();
	//d3.selectAll().remove;
	//d3.select("#mainsvg").remove();

}

function testPointArray(div, w, h){	
	
	//Dynamic, random dataset
	//var dataset = [];					//Initialize empty array
	var numDataPoints = 10;				//Number of dummy data points to create
	var xRange = Math.random() * 1000;	//Max range of new x values
	var yRange = Math.random() * 1000;	//Max range of new y values
	for (var i = 0; i < numDataPoints; i++) {					//Loop numDataPoints times
		var newNumber1 = Math.round(Math.random() * xRange);	//New random integer
		var newNumber2 = Math.round(Math.random() * yRange);	//New random integer
		datasetPoints.push([newNumber1, newNumber2]);					//Add new number to array
	}

	
	//Create scale functions for x and y axis
	var xScale = d3.scale.linear()
                         .domain([0, d3.max(datasetPoints, function(d) { return d[0]; })])
                         .range([padding, w - padding]); //w, the SVGÕs width. without padding 0,w
	
	var yScale = d3.scale.linear()
                         .domain([0, d3.max(datasetPoints, function(d) { return d[1]; })])// d[1], the y value of each sub-array. 
                         .range([h - padding, padding]); //without padding h,0
	
	//scales the radius of the circle
	var rScale = d3.scale.linear()
                         .domain([0, d3.max(datasetPoints, function(d) { return d[1]; })])
                         .range([2, 5]); // radius values will always fall within the range of 2,5
	
	//get the max number of values for the axis
	var axisXScale;
	var axisYScale;
	   if(d3.max(datasetPoints, function(d) { return d[0]; }) > d3.max(datasetPoints, function(d) { return d[1]; })){
		   axisXScale = xScale;
		   axisYScale = d3.scale.linear()
                                .domain([0, d3.max(datasetPoints, function(d) { return d[0]; })])
                                .range([h - padding, padding]);
	   }
	   else{
		   axisXScale = d3.scale.linear()
                                .domain([0, d3.max(datasetPoints, function(d) { return d[1]; })])
                                .range([padding, w - padding]);
		   axisYScale = yScale;
	   }
	
	//Define X axis
	var xAxis = d3.svg.axis()
					  .scale(xScale)
					  .orient("bottom")
					  .ticks(5);  //Set rough # of ticks;
	
	//Define Y axis
	var yAxis = d3.svg.axis()
	                  .scale(yScale)
	                  .orient("left")
	                  .ticks(5);
		
	//Create SVG element
	svg = d3.select(div)
				.append("svg")
				.attr("width", w)
				.attr("height", h);

	//Create circles
	svg.selectAll("circle")
	      .data(datasetPoints)
	      .enter()
	      .append("circle")
	      .attr("cx", function(d) {
	   		return xScale(d[0]);
	       })
	      .attr("cy", function(d) {
	   		return yScale(d[1]);
	      })
	      .attr("r", function(d) {
            return rScale(d[1]);
          });
	
	//Create X axis
	svg.append("g")
		.attr("class", "axis")
		.attr("transform", "translate(0," + (h - padding) + ")")
		.call(xAxis);
	
	//Create Y axis
	svg.append("g")
	    .attr("class", "axis")
	    .attr("transform", "translate(" + padding + ",0)")
	    .call(yAxis);
	
}

