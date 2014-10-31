/**
 * This file contains all javascript functions using the d3-library to draw elements in the graphical view.
 * The functions are called from the JSNI-methods in the graphical view and from the controller classes of 
 * each datatype.
 * 
 * @author Kristina Steiger
 */

//Width and height and padding of the svg
var w ;
var h ;
var padding = 55;
var svg ;

var xAxis ;
var yAxis ;

var outputRange = [];

//datasets for each implemented datatype
var datasetPoints = [];
var datasetLines = [];
var datasetPolylines = [];
var datasetPolygonPoints = [];//[x, y]
var datasetPolygons = []; //[[][]..]

//elements for moving points
var mpointPath = []; //[x, y]
var mpointTime = [];
var datasetMPoints = []; //[[][]..]
var duration = 10000;

/***********************************************************
*              Functions for Main SVG and Axes
***********************************************************/

/**Create the main SVG*/
function createSVG(div, width, height){
	
	w = height;
	h = height;

	svg = d3.select(div)
				.append("svg")
				.attr("id", "mainsvg")
				.attr("width", w)
				.attr("height", h);
}

/**Add a point to the output range*/
function addPointToOutputRange(x, y){
	var newNumber1 = x;	
	var newNumber2 = y;	
	outputRange.push([newNumber1, newNumber2]);	
}

/**Reset the range for the output and the axes*/
function resetOutputRange(){
	if (outputRange) {
	    for (i in outputRange) {
	      delete i;
	    }
    outputRange.length = 0;
    }
}

/**Create the SVG, scale the axes to the outputrange and draw the axes*/
function drawAxes(){
	
	//Create scale functions for x and y axis, get minimum and maximum of the first entry of the pointarray
	var xScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[0]; }), d3.max(outputRange, function(d) { return d[0]; })])
                         .range([padding, w - padding]); //w, the SVGs width, surrounded by a padding.
	
	var yScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[1]; }), d3.max(outputRange, function(d) { return d[1]; })])
                         .range([h - padding, padding]);
	
	
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
	
	//Define Y axis
	yAxis = d3.svg.axis()
	                  .scale(axisYScale)
	                  .orient("left")
	                  .ticks(5);
	
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

/**Remove the svg from the view*/
function removeSVG(){
	d3.select("svg").remove();

}

/**Remove all overlays from the svg*/
function removeAllOverlays(){
	d3.select("#mainsvg").selectAll("circle").remove();
	d3.select("#mainsvg").selectAll("line").remove();
	d3.select("#mainsvg").selectAll("polygon").remove();
	d3.select("#mainsvg").selectAll("path").remove();
}


/***********************************************************
*                Functions for Points
***********************************************************/

/**check if any points are in the array*/
function hasPoints(){

	  if (datasetPoints.length > 0) {
	    return true;
	  } else {
	    return false;
	  }
}

/**Add a point to the dataset for points*/
function addPointToDataset(x, y, text, id, color){
	var newNumber1 = x;	
	var newNumber2 = y;	
	var newText = text;
	datasetPoints.push([newNumber1, newNumber2, text, id, color]);	//Add new point to array
}

/**Show the point with the given id*/
function showPoint(id, color){
	d3.select("#" + "point" + id.toString()) // ids can't start with a number.
		.attr("r", 4)
	    .attr("fill", color);
}

/**Hide the point with the given id*/
function hidePoint(id){
	d3.select("#" + "point" + id.toString())
	    .attr("r", 0)
        .attr("fill", "white");
}

/**Draw given point to the output panel*/
function drawPoint(id, index, name, color){
	
	var point = datasetPoints[index];
	
	//Create scale functions for x and y axis
	var xScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[0]; }), d3.max(outputRange, function(d) { return d[0]; })])
                         .range([padding, w - padding]);
	
	var yScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[1]; }), d3.max(outputRange, function(d) { return d[1]; })])
                         .range([h - padding, padding]);
	
	//Create a circle with a tooltip
	 var circles = d3.select("#mainsvg")
	      .append("circle")
	      .attr("id", "point" + id)
	      .attr("cx", xScale(point[0]))
	      .attr("cy", yScale(point[1]))
	      .attr("r", 4)
          .attr("fill", color)
	      .on('mouseover', function(d){ d3.select(this).style({fill: 'red'}); })
          .on('mouseout', function(d){ d3.select(this).style({fill: color }); })
         .append("title")
         .text(name);	
}

/**Draw all points from the pointarray and scale them to the output panel*/
function showPointArray(){

	//Create scale functions for x and y axis
	var xScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[0]; }), d3.max(outputRange, function(d) { return d[0]; })])
                         .range([padding, w - padding]); 
	
	var yScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[1]; }), d3.max(outputRange, function(d) { return d[1]; })])
                         .range([h - padding, padding]);

	//Create circles with tooltips
	 var circles = d3.select("#mainsvg").selectAll("circle")
	      .data(datasetPoints)
	      .enter()
	      .append("circle")
	      .attr("id", function(d) { return "point" + d[3].toString(); 
          })
	      .attr("cx", function(d) {
	   		return xScale(d[0]);
	       })
	      .attr("cy", function(d) {
	   		return yScale(d[1]);
	      })
	      .attr("r", 4)
          .attr("fill", (function(d) { return d[4]; 
          }))
	      .on('mouseover', function(d){ d3.select(this).style({fill: 'red'}); })
          .on('mouseout', function(d){ d3.select(this).style({fill: (function(d) { return d[4]; 
          })}); })
          .append("title")
          .text(function(d) { return d[2]; 
           });
}

/**Delete all points in the array by removing references to them*/
function deleteAllPoints() {
	if (datasetPoints) {
	    for (i in datasetPoints) {
	      delete i;
	    }
    datasetPoints.length = 0;
  }
}

/**Remove all circles from the view*/
function removeCircles(){

	d3.select("#mainsvg").selectAll("circle").remove();
}

/**Remove the point with the given id*/
function removePoint(id){

	d3.select("#mainsvg").select("#" + "point" + id.toString()).remove();
}

/***********************************************************
*                Functions for Lines
***********************************************************/

/**Check if any lines are in the array*/
function hasLines(){

	  if (datasetLines.length > 0) {
	    return true;
	  } else {
	    return false;
	  }
}

/**Check if any polylines are in the array*/
function hasPolylines(){

	  if (datasetPolylines.length > 0) {
	    return true;
	  } else {
	    return false;
	  }
}

/**Add a line to the dataset for lines*/
function addLineToDataset(x1, y1, x2, y2){
	var newNumber1 = x1;	
	var newNumber2 = y1;	
	var newNumber3 = x2;
	var newNumber4 = y2;
	datasetLines.push([newNumber1, newNumber2, newNumber3, newNumber4]);//Add new line to array
}

/**Add polyline to the dataset for polylines*/
function addPolyline(){
	
    var polyline = [];
	
	for (i in datasetLines) {
	    polyline[i] = datasetLines[i];
	}
	datasetPolylines.push(polyline);
}

/**Show the polyline with the given id*/
function showPolyline(id){
	d3.select("#" + "polyline" + id.toString()) //ids can't start with a number.
		.attr("stroke-width", 2);
}

/**Hide the polyline with the given id*/
function hidePolyline(id){
	d3.select("#" + "polyline" + id.toString())
	    .attr("stroke-width", 0);
}

/**Draw all lines for one polyline from the linearray to the svg*/
function showLineArray(id, index, color){
	
	var lineArray = datasetPolylines[index];
    
    //Create scale functions for x and y axis
	var xScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[0]; }), d3.max(outputRange, function(d) { return d[0]; })])
                         .range([padding, w - padding]); 
	
	var yScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[1]; }), d3.max(outputRange, function(d) { return d[1]; })])
                         .range([h - padding, padding]);
	
   //Create lines
   var polylineGroup = d3.select("#mainsvg")
          .append("g")
          .attr("id", "polyline" + id)
          /* .on("mouseover", function() { d3.select(this).selectAll("line").style("stroke", "red");
           })
          .on("mouseout", function() { d3.select(this).selectAll("line").style("stroke", color);
           })*/
          .selectAll("line")
	      .data(lineArray)
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
         .attr("stroke", color);
         
}

/**Delete all lines in the array by removing references to them*/
function deleteAllLines() { 
	if (datasetLines) {
	    for (i in datasetLines) {
	      delete i;
	    }
    datasetLines.length = 0; 
  }
}

/**Delete all polylines in the polyline array*/
function deleteAllPolylines(){
	if (datasetPolylines) {
	    for (i in datasetPolylines) {
	      delete i;
	    }
	    datasetPolylines.length = 0;
  }
}

/**Remove all lines from the view*/
function removeLines(){
	d3.select("#mainsvg").selectAll("line").remove();
}

/**Remove the polyline with the given id*/
function removePolyline(id){
	d3.select("#mainsvg").select("#" + "polyline" + id.toString()).remove();
}

/***********************************************************
*                Functions for Polygons
***********************************************************/

/**Check if any polygons are in the array*/
function hasPolygons(){

	  if (datasetPolygons.length > 0) {
	    return true;
	  } else {
	    return false;
	  }
}

/**Add a point to the path for a polygon or polyline*/
function addPointToPolygonPath(x, y){
	var newNumber1 = x;	
	var newNumber2 = y;	
	datasetPolygonPoints.push([newNumber1, newNumber2]);
}

/**Add all points from the patharray as a polygon to the polygonarray*/
function addPolygon(text, id, color){	
	var polygon = [];
	
	for (i in datasetPolygonPoints) {
	    polygon[i] = datasetPolygonPoints[i];
	}
	datasetPolygons.push([polygon, text, id, color]);   
}

/**Show the polygon with the given id*/
function showPolygon(id, color){
	d3.select("#" + "polygon" + id.toString())
		.attr("fill", color)
        .attr("stroke-width", 1);
}

/**Hide the polygon with the given id*/
function hidePolygon(id){
	d3.select("#" + "polygon" + id.toString())//ids can't start with a number.
        .attr("fill", "white")
        .attr("stroke-width", 0);
}

/**Change the color of the polygon with the given id*/
function changePolygonColor(id, color){
	d3.select("#" + "polygon" + id.toString())
        .attr("fill", color);
}

/**Draw all points from the pointarray and scale them to the output panel*/
function drawPolygon(id, index, name, color){
	
	var pathArray = datasetPolygons[index];
	
	//Create scale functions for x and y axis
	var xScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[0]; }), d3.max(outputRange, function(d) { return d[0]; })])
                         .range([padding, w - padding]);
	
	var yScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[1]; }), d3.max(outputRange, function(d) { return d[1]; })])
                         .range([h - padding, padding]);
    
  //Create a path of points filled as a polygon	 
	d3.select("#mainsvg").selectAll("polygon")
	      .data([pathArray])
	      .enter()
	      .append("polygon")
	      .attr("id", "polygon" + id)
          .attr("points",function(d) { 
                       return d.map(function(d) {
                         return [xScale(d[0]),yScale(d[1])].join(",");
                     }).join(" ");
         }) 
         .attr("fill", color)
         .style('fill-opacity', .45)
         .attr("stroke-width", 1)
         .attr("stroke", "red")
         .on('mouseover', function(d){ d3.select(this).style('fill-opacity', .6); })
         .on('mouseout', function(d){ d3.select(this).style('fill-opacity', .45); })
         .append("title")
         .text(name);
}

/**Draw a path of all points for a polygon from the pathpointarray and scale them to the output panel*/
function showPolygonArray(){
	
	//Create scale functions for x and y axis
	var xScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[0]; }), d3.max(outputRange, function(d) { return d[0]; })])
                         .range([padding, w - padding]); 
	
	var yScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[1]; }), d3.max(outputRange, function(d) { return d[1]; })])
                         .range([h - padding, padding]);

	
	d3.select("#mainsvg").selectAll("polygon")
	            .data(datasetPolygons) 
	            .enter()
	            .append("polygon")
	            .attr("id", function(d) { return "polygon" + d[2].toString(); 
                 })
                .attr("points",function(d) { 
                     return d[0].map(function(d) { 
                     return [xScale(d[0]),yScale(d[1])].join(","); 
                  }).join(" ");
                }) 
               .attr("fill", function(d) { return d[3]; })
               .style('fill-opacity', .45)
               .attr("stroke-width", 1)
               .attr("stroke", "blue")
               .on('mouseover', function(d){ d3.select(this).style('fill-opacity', .6); })
               .on('mouseout', function(d){ d3.select(this).style('fill-opacity', .45); })
               .append("title")
               .text(function(d) { return d[1]; });
}

/**Delete all points in the array of the path*/
function deleteAllPolygonPoints(){
	if (datasetPolygonPoints) {
	    for (i in datasetPolygonPoints) {
	      delete i;
	    }
	    datasetPolygonPoints.length = 0;
  }
}

/**Delete all polygons in the polygon array*/
function deleteAllPolygons(){
	if (datasetPolygons) {
	    for (i in datasetPolygons) {
	      delete i;
	    }
	    datasetPolygons.length = 0;
  }
}

/**Remove all polygons from the view*/
function removePolygons(){
	d3.select("#mainsvg").selectAll("polygon").remove();
}

/***********************************************************
*                Functions for Moving Points
***********************************************************/

/**Check if any mpoints are in the array*/
function hasMPoints(){

	  if (mpointPath.length > 0) {
	    return true;
	  } else {
	    return false;
	  }
}

/**Add a point to the path for the moving point*/
function addPointToMPointPath(x, y){
	var newNumber1 = x;	
	var newNumber2 = y;	
	mpointPath.push([newNumber1, newNumber2]);
}

/**Add all points from the mpointpatharray as a path to the mpointarray*/
function addMPoint(){
	
	var mpoint = [];
	
	for (i in mpointPath) {
	    mpoint[i] = mpointPath[i];
	}	
	datasetMPoints.push(mpoint);	   
}

/**Draw the moving point with its path without moving it yet*/
function drawMovingPoint(id, index, color){
	
	var pathMPoint = datasetMPoints[index];
	
	//Create scale functions
	var xScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[0]; }), d3.max(outputRange, function(d) { return d[0]; })])
                         .range([padding, w - padding]); //w, the SVGs width. without padding 0,w
	
	var yScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[1]; }), d3.max(outputRange, function(d) { return d[1]; })])
                         .range([h - padding, padding]); //without padding h,0
    
    //This is the line accessor function
    var lineFunction = d3.svg.line()
                           .x(function(d) {
   		                        return xScale(d[0]);
                                   })
                           .y(function(d) {
   		                        return yScale(d[1]);
                                   })
                           .interpolate("linear");
    
	
    var path =  d3.select("#mainsvg").append("path") 
          .attr("id", "mpath" + id)
          .attr("d", lineFunction(pathMPoint))
          .attr("fill", "none")
          .attr("stroke-width", 2)
          .attr("stroke", "steelblue");
	
	var cx = xScale(pathMPoint[0][0]);
	var cy = yScale(pathMPoint[0][1]);

	 //add moving point to the beginning of the path
	var movingpoint = d3.select("#mainsvg").append("circle")
	      .attr("id", "mpoint" + id)
	      .attr("cx", cx)
	      .attr("cy", cy)
          .attr("r", 8)
          .attr("fill", color);
}


/**Draw all data from the mpointarray, scale them to the output panel and move the point along the path*/
function animateMovingPoint(id, index, color){
	
	var pathMPoint = datasetMPoints[index];

	//Create scale functions
	var xScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[0]; }), d3.max(outputRange, function(d) { return d[0]; })])
                         .range([padding, w - padding]); 
	
	var yScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[1]; }), d3.max(outputRange, function(d) { return d[1]; })])
                         .range([h - padding, padding]);

    
    //This is the line accessor function
    var lineFunction = d3.svg.line()
                           .x(function(d) {
   		                        return xScale(d[0]);
                                   })
                           .y(function(d) {
   		                        return yScale(d[1]);
                                   })
                           .interpolate("linear");
    
	
    var path =  d3.select("#mainsvg").append("path")
          .attr("id", "mpath" +  id)
          .attr("d", lineFunction(pathMPoint))
          .attr("fill", "none")
          .attr("stroke-width", 2)
          .attr("stroke", "steelblue");
          

    //create moving point
	var movingpoint = d3.select("#mainsvg").append("circle") 
	     .attr("id", "mpoint" + id)
         .attr("r", 8)
         .attr("fill", color)
         .on("mousedown", transition);

    transition();

    function transition() {
    	movingpoint.transition()
            .ease( "linear" )
            .duration(duration)
            .attrTween("transform", translateAlong(path.node()));
     }

   // Returns an attrTween for translating along the specified path element.
    function translateAlong(path) {
        var l = path.getTotalLength();
        return function(d, i, a) {
         return function(t) {
           var p = path.getPointAtLength(t * l);
          return "translate(" + p.x + "," + p.y + ")";
          };
        };
      }
}

/**Move the point to the given position*/
function movePoint(id, x, y){
	
	//Create scale functions
	var xScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[0]; }), d3.max(outputRange, function(d) { return d[0]; })])
                         .range([padding, w - padding]);
	
	var yScale = d3.scale.linear()
                         .domain([d3.min(outputRange, function(d) { return d[1]; }), d3.max(outputRange, function(d) { return d[1]; })])
                         .range([h - padding, padding]); 
	
	var cx = xScale(x);
	var cy = yScale(y);
	
	d3.select("#" + "mpoint" + id)
          .transition()
          .attr("cx", cx)
          .attr("cy", cy)
	      .duration(50);
}

/**Remove to given moving point from the view*/
function removeMovingPoint(id){
	d3.select("#mpoint" + id).remove();
	d3.select("#mpath" + id).remove();
}

/**Change the color of the moving point with the given id*/
function changeMPointColor(id, color){
	d3.select("#" + "mpoint" + id.toString())
        .attr("fill", color);
}

/**Delete all pathpoints from the mpoint path array*/
function deletePathOfMPoint(){
	if (mpointPath) {
	    for (i in mpointPath) {
	      delete i;
	    }
    mpointPath.length = 0;
    }
}

/**Delete all mpoint paths in the array of the mpoints*/
function deleteAllMPointPaths(){
	if (datasetMPoints) {
	    for (i in datasetMPoints) {
	      delete i;
	    }
	    datasetMPoints.length = 0;
  }
}

///**handle fileUpload-input element*/
//function handleInput(ev) {
//	window.alert(ev.target.value);
//	Element el=document.getElementById("gwt-debug-fileUploadHidden");
//	window.alert("On input"+el.value);
//    document.getElementById("gwt-debug-textBoxForUpload").value=el.value; 
//}
