function d3_barchart(div, data) {
  
  var x = d3.scale.linear().domain([0, d3.max(data)]).range(["0px", "420px"]);
  
  var chart = d3.select(div).append("div").attr("class", "chart");
     
  chart.selectAll("div").data(data)
       .enter().append("div").style("width", x)
       .text(function(d) { return d; });
     
}

function d3_shapes_test(div) {
	
	//var svgSelection = d3.select(div);
	 

	 var circle = d3.select(div)
     .append("svg")
     .attr("width", 100)
     .attr("height", 100);    

    circle.append("circle")
     .style("stroke", "gray")
     .style("fill", "white")
     .attr("r", 40)
     .attr("cx", 50)
     .attr("cy", 50)
     .on("mouseover", function(){d3.select(this).style("fill", "aliceblue");})
     .on("mouseout", function(){d3.select(this).style("fill", "white");})
     .on("mousedown", animateFirstStep);

     function animateFirstStep() {
         d3.select(this).transition()
             .duration(1000)
             .attr("r", 10)
             .each("end", animateSecondStep);
     };

     function animateSecondStep(){
         d3.select(this)
           .transition()
             .duration(1000)
             .attr("r", 40);
     };
    
     /**animation of a square going first right then down on click*/
    var mySquare = d3.select(div)
     .append("svg")
     .attr("width", 350)
     .attr("height", 350);   
    
    // add a square
    mySquare.append("rect")
	    .attr("x", 60)
	    .attr("y", 60)
	    .attr("width", 60)
	    .attr("height", 60)
	    .style("fill", "red")
        .on("mouseover", function(){d3.select(this).style("fill", "blue");})
        .on("mouseout", function(){d3.select(this).style("fill", "red");})
        .on("mousedown", animate1);

     function animate1() {
         d3.select(this).transition()
            .attr("x",200)
	        .duration(1000) // this is 1s
	        .delay(100)    // this is 0.1s
	        .each("end", animate2);
     };

     //erst wenn die erste Animation beendet ist wird die zweite gestartet
     function animate2(){
         d3.select(this).transition()
            .attr("y",200)
	        .duration(1000) // this is 1s
	        .delay(100) ;
     };


	// moves the square when clicking on the button
	/*var button = d3.select(div)
	   .attr("type", "button")
	   .attr("name", "mybutton");
	   .attr("value", "click!");*/

	   
	//button.on("click", function() { mySquare.transition().attr("x", 320); })*/
}


function d3_lines_test(div){
	
	/**a line*/
    var myLine = d3.select(div)
     .append("svg")
     .attr("width", 200)
     .attr("height", 200);   
    
    // add a line
    myLine.append("line")
	    .attr("x1", 5)
        .attr("y1", 5)
        .attr("x2", 50)
        .attr("y2", 50)
        .attr("stroke-width", 2)
        .attr("stroke", "black");
    
    /**a polyline*/
    var myPolyline = d3.select(div)
     .append("svg")
     .attr("width", 200)
     .attr("height", 200);   
    
    // add a polyline
    myPolyline.append("polyline")
	    .attr("points", "05,50 25,50 25,20 35,20 35,05 50,05")
        .attr("stroke-width", 2)
        .attr("fill", "none")
        .attr("stroke", "green")
        .on("mouseover", animate1);

    function animate1() {
        d3.select(this).transition()
            .attr("points", "10,20 18,50 1,20 20,40 35,20 25,60")
	        .duration(1000) // this is 1s
	        .delay(100)    // this is 0.1s
	        .each("end", animate2);
    };

    //erst wenn die erste Animation beendet ist wird die zweite gestartet
    function animate2(){
        d3.select(this).transition()
            .attr("points", "05,50 25,50 25,20 35,20 35,05 50,05")
	        .duration(1000) // this is 1s
	        .delay(100) ;
    };
    
    
	/**a polygon*/
    var myPolygon = d3.select(div)
     .append("svg")
     .attr("width", 200)
     .attr("height", 200);   
    
    // add a polygon
    myPolygon.append("polygon")
        .attr("points", "50,5 100,5 125,30 125,80 100,105 50,105 25,80 25, 30")
        .attr("fill", "green")
        .attr("stroke-width", 2)
        .attr("stroke", "red")
        .on("mouseover", function(){d3.select(this).style("fill", "blue");})
        .on("mouseout", function(){d3.select(this).style("fill", "green");});
	
}

function d3_path_test(div){
	
	/**a path*/
    var myPathpanel = d3.select(div)
     .append("svg")
     .attr("width", 200)
     .attr("height", 200);   
    
    // add a path that draws a triangle: M = moves pen to point without drawing, L= draws line Z= returns to starting point
    myPathpanel.append("path")
        .attr("d", "M 10 25  L 10 75  L 60 75 L 10 25")
        .attr("fill", "none")
        .attr("stroke-width", 2)
        .attr("stroke", "green");
    
    /**path data generation - line example*/
  //The data for the line in an array
    var lineData = [ { "x": 1,   "y": 5},  { "x": 20,  "y": 20},
                      { "x": 40,  "y": 10}, { "x": 60,  "y": 40},
                      { "x": 80,  "y": 5},  { "x": 100, "y": 60}];
   
    //This is the accessor function
    var lineFunction = d3.svg.line()
                       .x(function(d) { return d.x; })
                       .y(function(d) { return d.y; })
                       .interpolate("linear");
    
    /**draw the path on a panel*/
    var myPathpanel2 = d3.select(div)
     .append("svg")
     .attr("width", 200)
     .attr("height", 200);   
    
    // add a path that draws a path from a line function
    myPathpanel2.append("path")
                .attr("d", lineFunction(lineData))
                .attr("stroke", "blue")
                .attr("stroke-width", 2)
                .attr("fill", "none");
    
    /**Getting circles from a json-object and fill 3 circles dynamically*/
    var jsonCircles = [{ "x_axis": 30, "y_axis": 30, "radius": 20, "color" : "green" },
                       { "x_axis": 70, "y_axis": 70, "radius": 20, "color" : "purple"},
                       { "x_axis": 110, "y_axis": 100, "radius": 20, "color" : "red"}];
    
    var myPathpanel3 = d3.select(div)
       .append("svg")
       .attr("width", 200)
       .attr("height", 200);   
   
   // add a path that draws a path from a line function
    var circles = myPathpanel3.selectAll("circle")
                              .data(jsonCircles)
                              .enter()
                              .append("circle");
    
    var circleAttributes = circles
                           .attr("cx", function (d) { return d.x_axis; })
                           .attr("cy", function (d) { return d.y_axis; })
                           .attr("r", function (d) { return d.radius; })
                           .style("fill", function(d) { return d.color; });
}

function d3_berlin_test(div) {
	
	 var mySquare = d3.select(div)
     .append("svg")
     .attr("width", "100%")
     .attr("height", "100%");    
    
    // add a square for a point
    mySquare.append("rect")
	    .attr("x", 93.960)
	    .attr("y", 98.710)
	    .attr("width", 10)
	    .attr("height", 10)
	    .style("fill", "red")
        .on("mouseover", function(){d3.select(this).style("fill", "blue");})
        .on("mouseout", function(){d3.select(this).style("fill", "red");})
        .on("mousedown", animate1);

     function animate1() {
         d3.select(this).transition()
            .attr("x",68.450)
	        .duration(1000) // this is 1s
	        .delay(100)    // this is 0.1s
	        .each("end", animate2);
     };

     //erst wenn die erste Animation beendet ist wird die zweite gestartet
     function animate2(){
         d3.select(this).transition()
            .attr("y",47.360)
	        .duration(1000) // this is 1s
	        .delay(100) ;
     };

}


