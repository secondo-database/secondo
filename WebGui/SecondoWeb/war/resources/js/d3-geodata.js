
//Width and height of the svg
var w ;
var h ;
var padding = 45;
var svg ;
var xAxis ;
var yAxis ;

var datasetPoints = [];
var datasetLines = [];
var datasetPathPoints = [];
var datasetPaths = [];
var datasetPolygons = [];

var outputRange = [];

/**check if any points are in the array*/
function hasPoints(){

	  if (datasetPoints.length > 0) {
		  //alert("number of points in dataset: " + datasetPoints.length);
	    return true;
	  } else {
	    return false;
	  }
}

/**check if any lines are in the array*/
function hasLines(){

	  if (datasetLines.length > 0) {
		 // alert("number of lines in dataset: " + datasetLines.length);
	    return true;
	  } else {
	    return false;
	  }
}

/**check if any polygons are in the array*/
function hasPolygons(){

	  if (datasetPolygons.length > 0) {
		 // alert("number of polygons in dataset: " + datasetPolygons.length);
	    return true;
	  } else {
	    return false;
	  }
}

/**Creates the main SVG*/
function createSVG(div, width, height){
	
	w = height;
	h = height;
	//Create SVG element
	svg = d3.select(div)
				.append("svg")
				.attr("id", "mainsvg")
				.attr("width", w)
				.attr("height", h);
}

/**Resize the SVG to the given size*/
function resizeSVG(height){
	
	d3.select("#mainsvg").attr("width", height).attr("height", height);
}


/**Creates the SVG, scales the axes to the outputrange and draws the axes*/
function drawAxes(){
	
	//Create scale functions for x and y axis
	var xScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[0]; }), d3.max(outputRange, function(d) { return d[0]; })])//get minimum and maximum of the first entry of the pointarray
                         .range([padding, w - padding]); //w, the SVGs width. without padding 0,w
	
	var yScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[1]; }), d3.max(outputRange, function(d) { return d[1]; })])
                         .range([h - padding, padding]); //without padding h,0
	
	
	//get the max number of values for the axis
	var axisXScale;
	var axisYScale;
	   if(d3.max(outputRange, function(d) { return d[0]; }) > d3.max(outputRange, function(d) { return d[1]; })){
		   axisXScale = xScale;
		   axisYScale = d3.scale.linear()
                                .domain([d3.min(outputRange, function(d) { return d[0]; }), d3.max(outputRange, function(d) { return d[0]; })])
                                .range([h - padding, padding]);
	   }
	   else{
		   axisXScale = d3.scale.linear()
                                .domain([d3.min(outputRange, function(d) { return d[1]; }), d3.max(outputRange, function(d) { return d[1]; })])
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
	
	//Create X axis
	d3.select("#mainsvg").append("g")
		.attr("class", "x axis")
		.attr("shape-rendering", "crispEdges")
		.attr("transform", "translate(0," + (h - padding) + ")")// transform the entire axis group (g), pushing it to the bottom:
		.call(xAxis);
	
	//Create Y axis
	d3.select("#mainsvg").append("g")
	    .attr("class", "y axis")
	    .attr("shape-rendering", "crispEdges")
	    .attr("transform", "translate(" + padding + ",0)")
	    .call(yAxis);
}

/**Update the axes*/
function updateAxes(){ //funktioniert nicht :(
	//Create scale functions for x and y axis
	var xScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[0]; }), d3.max(outputRange, function(d) { return d[0]; })])//get minimum and maximum of the first entry of the pointarray
                         .range([padding, w - padding]); //w, the SVGs width. without padding 0,w
	
	var yScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[1]; }), d3.max(outputRange, function(d) { return d[1]; })])
                         .range([h - padding, padding]); //without padding h,0
	
	
	//get the max number of values for the axis
	var axisXScale;
	var axisYScale;
	   if(d3.max(outputRange, function(d) { return d[0]; }) > d3.max(outputRange, function(d) { return d[1]; })){
		   axisXScale = xScale;
		   axisYScale = d3.scale.linear()
                                .domain([d3.min(outputRange, function(d) { return d[0]; }), d3.max(outputRange, function(d) { return d[0]; })])
                                .range([h - padding, padding]);
	   }
	   else{
		   axisXScale = d3.scale.linear()
                                .domain([d3.min(outputRange, function(d) { return d[1]; }), d3.max(outputRange, function(d) { return d[1]; })])
                                .range([padding, w - padding]);
		   axisYScale = yScale;
	   }
	
	//Define X axis
	xAxis = d3.svg.axis()
					  .scale(axisXScale)
					  .orient("bottom");
					  //.ticks(5);  //Set rough # of ticks;
	                  //.tickFormat(commasFormatter);
	
	//Define Y axis
	yAxis = d3.svg.axis()
	                  .scale(axisYScale)
	                  .orient("left");
	                  //.ticks(5);
	                  //.tickFormat(formatTick);
	

	d3.select("#mainsvg").select(".x.axis")
	  .attr("shape-rendering", "crispEdges")
      .attr("transform", "translate(0," + (h - padding) + ")")
      .call(xAxis);
   
	d3.select("#mainsvg").select(".y.axis")
	  .attr("shape-rendering", "crispEdges")
	  .attr("transform", "translate(" + padding + ",0)")
      .call(yAxis)

}

/**draw all points from the pointarray and scale them to the output panel*/
function showPointArray(){

	//Create scale functions for x and y axis
	var xScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[0]; }), d3.max(outputRange, function(d) { return d[0]; })])//get minimum and maximum of the first entry of the pointarray
                         .range([padding, w - padding]); //w, the SVGs width. without padding 0,w
	
	var yScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[1]; }), d3.max(outputRange, function(d) { return d[1]; })])
                         .range([h - padding, padding]); //without padding h,0
	
	//scales the radius of the circle
	var rScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[1]; }), d3.max(outputRange, function(d) { return d[1]; })])
                         .range([2, 5]); // radius values will always fall within the range of 2,5

	//Create circles
	 d3.select("#mainsvg").selectAll("circle")
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
	
}

/**Draw all lines from the linearray to the svg*/
function showLineArray(){
	
	//check which scale is the biggest one and take this one for the axis and range of points
	//get the max and min number of values for the axis	
	
	/*var maximumArray = [d3.max(datasetLines, function(d) { return d[0]; }), d3.max(datasetLines, function(d) { return d[1]; }), 
			            d3.max(datasetLines, function(d) { return d[2]; }), d3.max(datasetLines, function(d) { return d[3]; })];
	
	var minimumArray = [d3.min(datasetLines, function(d) { return d[0]; }), d3.min(datasetLines, function(d) { return d[1]; }), 
			            d3.min(datasetLines, function(d) { return d[2]; }), d3.min(datasetLines, function(d) { return d[3]; })];
			             
    var maximum = d3.max(maximumArray);  
    var minimum = d3.min(minimumArray);

    var indexMaximum = maximumArray.indexOf(maximum); 
    var indexMinimum = minimumArray.indexOf(minimum); 
    
    var xScale = d3.scale.linear()
                         .domain([d3.min(datasetLines, function(d) { return d[indexMinimum]; }), d3.max(datasetLines, function(d) { return d[indexMaximum]; })])
                         .range([padding, w - padding]);
    var yScale = d3.scale.linear()
                         .domain([d3.min(datasetLines, function(d) { return d[indexMinimum]; }), d3.max(datasetLines, function(d) { return d[indexMaximum]; })])
                         .range([h - padding, padding]);*/
    
  //Create scale functions for x and y axis
	var xScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[0]; }), d3.max(outputRange, function(d) { return d[0]; })])//get minimum and maximum of the first entry of the pointarray
                         .range([padding, w - padding]); //w, the SVGs width. without padding 0,w
	
	var yScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[1]; }), d3.max(outputRange, function(d) { return d[1]; })])
                         .range([h - padding, padding]); //without padding h,0
		
	//var formatTick = d3.format(".1");
    //var commasFormatter = d3.format(",.0f");//will display integers with comma-grouping for thousands
	
	//Define X axis
	/*xAxis = d3.svg.axis()
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
	    .call(yAxis);*/
		

	//Create lines
    d3.select("#mainsvg").selectAll("line")
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

}

/**Add all points from the patharray as a polygon to the polygonarray*/
function addD3Polygon(){
	
	var polygon = [];
	
	for (i in datasetPathPoints) {
	    polygon[i] = datasetPathPoints[i];
	}
	
	datasetPolygons.push(polygon);
	   
}

/**Draw all points from the pointarray and scale them to the output panel*/
function drawD3Polygon(){
	
	//Create scale functions for x and y axis
	var xScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[0]; }), d3.max(outputRange, function(d) { return d[0]; })])//get minimum and maximum of the first entry of the pointarray
                         .range([padding, w - padding]); //w, the SVGs width. without padding 0,w
	
	var yScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[1]; }), d3.max(outputRange, function(d) { return d[1]; })])
                         .range([h - padding, padding]); //without padding h,0
	
	//This is the accessor function
    /*var lineFunction = d3.svg.line()
                       .x(function(d) {
               	   		return xScale(d[0]);
            	        })
                       .y(function(d) {
               	   		return yScale(d[1]);
             	        })
                       .interpolate("linear");
    
    d3.select("svg")
          .append("path")
          .attr("fill", "none")
          .attr("stroke", "black")
          .attr("d", lineFunction(datasetPathPoints)); //funktioniert*/
    
  //Create a path of points filled as a polygon	 
	d3.select("#mainsvg").selectAll("polygon")
	      .data([datasetPathPoints])//draws just one polygon
	      .enter()
	      .append("polygon")
         .attr("points",function(d) { 
                 return d.map(function(d) { 
                  return [xScale(d[0]),yScale(d[1])].join(",");
                 }).join(" ");
                }) 
         .attr("fill", "green")
         .style('fill-opacity', .15)
         .attr("stroke-width", 2)
         .attr("stroke", "red");

}

/**draw all points from the pointarray and scale them to the output panel*/
function drawPolygonIntern(pointarray){

	//Create scale functions
	var xScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[0]; }), d3.max(outputRange, function(d) { return d[0]; })])//get minimum and maximum of the first entry of the pointarray
                         .range([padding, w - padding]); //w, the SVGs width. without padding 0,w
	
	var yScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[1]; }), d3.max(outputRange, function(d) { return d[1]; })])
                         .range([h - padding, padding]); //without padding h,0
	
    
  //Create a path of points filled as a polygon	 
	   d3.select("#mainsvg").selectAll("polygon")
	      .data([pointarray])//draws just one polygon
	      .enter()
	      .append("polygon")
         .attr("points",function(d) { 
                 return d.map(function(d) { 
                  return [xScale(d[0]),yScale(d[1])].join(",");
                 }).join(" ");
                }) 
         .attr("fill", "green")
         .style('fill-opacity', .15)
         .attr("stroke-width", 2)
         .attr("stroke", "red");

}

/**draw a path of all points from the pathpointarray and scale them to the output panel*/
function showPolygonArray(){
	
	//Create scale functions for x and y axis
	var xScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[0]; }), d3.max(outputRange, function(d) { return d[0]; })])//get minimum and maximum of the first entry of the pointarray
                         .range([padding, w - padding]); //w, the SVGs width. without padding 0,w
	
	var yScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[1]; }), d3.max(outputRange, function(d) { return d[1]; })])
                         .range([h - padding, padding]); //without padding h,0
    
	//alert("length of datasetPolygons: " +datasetPolygons.length); //4,korrekt
         
	//Create a path of points filled as a polygon		
	/*for (var i = 0; i < datasetPolygons.length; i++) {
		
		drawPolygonIntern(datasetPolygons[i]);  //creates only the first polygon?? durchlŠuft die schleife nicht?, das gleiche wie bei variante unten */ 
	
	d3.select("#mainsvg").selectAll("polygon")
	            .data([datasetPolygons]) //draws only one polygon	 
	            .enter()
	            .append("polygon")
                .attr("points",function(d) { 
                     return d[0].map(function(d) { 
                     return [xScale(d[0]),yScale(d[1])].join(","); 
                  }).join(" ");
                }) 
               .attr("fill", "green")
               .style('fill-opacity', .15)
               .attr("stroke-width", 2)
               .attr("stroke", "red");
	//}

}


/**Adds a point to the dataset*/
function addPointToDataset(x, y){
	var newNumber1 = x;	
	var newNumber2 = y;	
	datasetPoints.push([newNumber1, newNumber2]);	//Add new number to array
}

/**Adds a line to the dataset*/
function addLineToDataset(x1, y1, x2, y2){
	var newNumber1 = x1;	
	var newNumber2 = y1;	
	var newNumber3 = x2;
	var newNumber4 = y2;
	datasetLines.push([newNumber1, newNumber2, newNumber3, newNumber4]);	//Add new numbers to array
}

/**Adds a point to the path for a polygon or polyline*/
function addPointToPath(x, y){
	var newNumber1 = x;	
	var newNumber2 = y;	
	datasetPathPoints.push([newNumber1, newNumber2]);	//Add new number to array
}

function addPointToOutputRange(x, y){
	var newNumber1 = x;	
	var newNumber2 = y;	
	outputRange.push([newNumber1, newNumber2]);	//Add new number to array
}

function resetOutputRange(){
	if (outputRange) {
	    for (i in outputRange) {
	      delete i;
	    }
    outputRange.length = 0;
    }
}

/**Deletes all points in the array by removing references to them*/
function deleteAllPoints() {
	if (datasetPoints) {
	    for (i in datasetPoints) {
	      delete i;
	    }
    datasetPoints.length = 0;
    //alert("Points deleted. Size of array: " + datasetPoints.length); immer 0
  }
}

/**Deletes all points in the array by removing references to them*/
function deleteAllD3Lines() { 
	if (datasetLines) {
	    for (i in datasetLines) {
	      delete i;
	    }
    datasetLines.length = 0; 
    //alert("Lines deleted. Size of array: " + datasetLines.length); immmer 0
  }
}

/**Deletes all points in the array of the path*/
function deleteAllPathPoints(){
	if (datasetPathPoints) {
	    for (i in datasetPathPoints) {
	      delete i;
	    }
	    datasetPathPoints.length = 0;
  }
}

/**Deletes all points in the array of the path*/
function deleteAllD3Polygons(){
	if (datasetPolygons) {
	    for (i in datasetPolygons) {
	      delete i;
	      //datasetPolygons.splice(i,1);
	    }
	    datasetPolygons.length = 0;
  }
}

/**Removes the svg from the view*/
function removeSVG(){
	d3.select("svg").remove();

}

/**Removes all overlays from the svg*/
function removeOverlays(){
	d3.select("#mainsvg").selectAll(".circle").remove();
	d3.select("#mainsvg").selectAll(".line").remove();
	d3.select("#mainsvg").selectAll(".polygon").remove();
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

