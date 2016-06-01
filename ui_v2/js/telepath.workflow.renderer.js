(function(){
	/*
	 * Function: Renderer
	 * 
	 * Invoked:
	 * 	From "Halfviz.js". The main function is ran once as we load the workflow panel for the first time.
	 * 	The function is ran and returns the "that" object. "That" contains two main functions.
	 * 	Init- returns
	 * 
	 * Parameters:
	 * 
	 * 
	 * Returns:
	 * 	an arborjs object with two main method: init and redraw.
	 * 
	 * See Also:
	 * 	<init>
	 * 	<redraw>
	 */
  
	Renderer = function(canvas){
		var canvas = $(canvas).get(0)	//the canvas object
		var ctx = canvas.getContext("2d"); //the canvas context object
		var gfx = arbor.Graphics(canvas)	//arbor object
		var canvas_x_offset = window.parent.Ext.getCmp('w_diagram').getPosition()[0] //the start x position for the canvas
		var particleSystem = null	//particle system object of arbor
		var nodeBoxes = {};	//saves the box size for each 
		/*
		 * Variable: Renderer.that (return value)
		 * 		contains two main methods - init, redraw
		 * 		
		 * See Also:
		 * 	<init>
		 * 	<redraw>
		 */
    		var that = {
    			ctx:ctx,
			gfx:gfx,
			node_hovered:undefined,
			node_rightclicked:undefined,
			canvas:canvas,
			handler:undefined,
			canvas_x_offset:canvas_x_offset,
			particleSystem:undefined,
			/*
			 * Function: init
			 * 
			 * Invoked:
			 * 	The constructor of the graph. Invoked every time a node is removed/added or an edge is removed/added.
			 * 	NOT invoked when nodes move
			 * 
			 * Parameters:
			 * 	system - the particle system created in halfviz.js
			 * 
			 * Returns:
			 * 	undefined
			 * 
			 * See Also:
			 * 	<init>
			 * 	<redraw>
			 */	
      			init:function(system){
				particleSystem = system;
				particleSystem.screenSize(canvas.width, canvas.height);
				particleSystem.screenPadding(100);
				that.initMouseHandling();
      			},
			/*
			 * Function: redraw
			 * 
			 * Invoked:
			 * 	The main function of the graph. The function is called by arbor mechanism everytime a node moves.
			 * 	(The function is invoked thousands of time).Everytime the arbor mechanism wants to inform that an edge moved in a pixel
			 * 	or a node needs to be redrawn. the new coordinates are updated in each node's properties _pt that has x and y
			 * 	All we need to do is take those new coordinates rendered by arbor's algorithm and draw the nodes accordingly 
			 * 	on the canvas along with the edges
			 * 
			 * Parameters:
			 * 	system - the particle system created in halfviz.js
			 * 
			 * Returns:
			 * 	undefined
			 * 
			 * See Also:
			 * 	<draw_node>
			 * 	<draw_edge>
			 * 
			 */
	      		redraw:function(){
        			if (!particleSystem) return
       				gfx.clear() // convenience ƒ: clears the whole canvas rect
        			// draw the nodes & save their bounds for edge drawing
        			nodeBoxes = {}
				//go on all nodes
        			particleSystem.eachNode(function(node, pt){
					//if a node doesnt have a label, it's a garbage node, fix bug by removing it
					if (node.data.label==undefined){
						particleSystem.pruneNode(node)
					}
					else{
						//redraw the node in his new position
						this.renderer.draw_node(node,pt,ctx,gfx);
						
						//reposition the menus associated with the node if the menus are visible
						if (	that.node_hovered && 
							that.node_hovered.name==node.name && 
							window.parent.node_menu_horizontal.isVisible() &&
							node.data.menu_visible	//&&
							//window.parent.w_pages_add_mode!=undefined
						) {
							window.parent.node_menu_horizontal.setPosition(pt.x+canvas_x_offset-node.data.width*0.33, pt.y+105)

						}
						if (	that.node_rightclicked &&
							that.node_rightclicked.data.id == node.data.id && 
							parent.w_rightclick_menu.isVisible()
						) {
							parent.w_rightclick_menu.setPosition(pt.x+canvas_x_offset-node.data.width*0.33, pt.y+55)
						}
						
						
					}

        			})    			


				// draw the edges
				particleSystem.eachEdge(function(edge, pt1, pt2){
					that.draw_edge(edge,pt1,pt2)
				})

			},//end redraw
			/*
			 * Function: draw_node
			 * 
			 * Invoked:
			 * 	The main function to "move" a node from one position to another.
			 * 	The function takes the fields of the node object and draws the node accordingly
			 * 	i.e if it has indices in a group, we will draw the indices in red above the node
			 * 	    if it's hovered we'll highlight the node's frame
			 * 	    if it's in a group, we'll draw the frame in red etc
			 * 
			 * Parameters:
			 * 	node - the node object to redraw
			 * 	pt - point object with the new coordinates
			 * 	ctx - the canvas context
			 * 	gfx - arbor object
			 * 	canvas_x_offset - the x start position of the canvas on the page
			 * 
			 * Returns:
			 * 	undefined
			 * 
			 * See Also:
			 * 	<init>
			 * 	<redraw>
			 * 
			 */
			draw_node:function(node,pt,ctx,gfx,canvas_x_offset){
				// node: {mass:#, p:{x,y}, name:"", data:{}}
				// pt:   {x:#, y:#}  node position in screen coords
				// determine the box size and round off the coords if we'll be 
				// drawing a text label (awful alignment jitter otherwise...)
				var label = (node.data.hovered?node.data.full_label:node.data.label);
				//prepare the node's width to be, according to the text width
				var w = ctx.measureText(""+label).width
				//if it has an image, the image's width is 30
				var img_w = 30
				w=w+10 //for padding
				//if the node has an icon, increase the node's width
				if (node.data.has_icon)
					w=w+img_w
				var indices = node.data.indices_in_group
				//if the node is in a workflow saved group,the width of every index square is the width of that number +10px
				if (indices.length>0)
					var w_index = ctx.measureText(""+indices[0]).width + 10
				//assign the width to the node's width property
				node.data.width = w
				if (!(""+label).match(/^[ \t]*$/)){
					pt.x = Math.floor(pt.x)
					pt.y = Math.floor(pt.y)
	  			}
				else{
	    				label = null
	  			}
				//draw a rectangle around the points of that node
				roundRect(ctx,pt.x -w/2, pt.y -16 , w,30);
				//save the measures in the nodeBoxes object
				nodeBoxes[node.name] = [pt.x-w/2, pt.y-16, w, 30]
				//if the node is selected, draw in red, make line thicker,
				if (node.data.selected){
					clean_node(node,pt,w,ctx)
					ctx.lineWidth = 3
					ctx.strokeStyle = '#FF4040'
				}
				//if the node is hovered, draw in red, make line thicker,
				else if(node.data.hovered){
					clean_node(node,pt,w,ctx)
					ctx.lineWidth = 3
					ctx.strokeStyle = '#04408C'
				}
				//if the node is neither hovered nor selected, draw in blue, make line thin
	  			else{
	  				//bug fix for hovering out of expanded shortened label
					//if the label is shortened due to long name, clean the node with his previous size(the one of the full label)
	  				if (node.data.full_label!=node.data.label){
						var w_tmp = ctx.measureText(""+node.data.full_label).width
						var img_w = 30
						w_tmp=w_tmp+10 //for padding
						if (node.data.has_icon)
							w_tmp=w_tmp+img_w //for the json icon
						clean_node(node,pt,w_tmp,ctx)
						
	  				}
					clean_node(node,pt,w,ctx)
					ctx.lineWidth = 1
	  				ctx.strokeStyle ="#04408C"
	  			}
				//if the node has an icon attached, draw the icon
				if (node.data.has_icon){
					if (node.data.image_obj==undefined){
						node.data.image_obj = new Image();
					}
					//draw image in the rectangle
					node.data.image_obj.onload = function() {
						node.data.image_obj_loaded=true
					}
					if (node.data.image_obj_loaded){
						ctx.clearRect(pt.x-w/2+2, pt.y -14, img_w-5, img_w-5);
						ctx.drawImage(node.data.image_obj, pt.x-w/2+2,pt.y -14,img_w-5,img_w-5);					
					}				
					node.data.image_obj.src = node.data.icon_src;				
				}				
				//redraw with appropriate color
				roundRect(ctx,pt.x -w/2, pt.y -16 , w,30);
	  			//draw the text inside the box of the node
	  			if (label){
	  				var node_size = 3*2+10;
					ctx.font = " "+node_size+"px Tahoma,Arial,Verdana"
					ctx.textAlign = "center"
					ctx.fillStyle = "#04408C"
					ctx.lineWidth = 1
					if (node.data.color=='none') ctx.fillStyle = '#333333'
					//if has icon move the text to the left
					if (node.data.has_icon)
						ctx.fillText(label, pt.x+10, pt.y+4)
					else
						ctx.fillText(label, pt.x, pt.y+4)
	  			}
				//if the node is associated with a group, also add the index above it
				if (node.data.selected){
					ctx.lineWidth = 1
					ctx.strokeStyle = '#FF4040'				
	  				var node_size = 3*2+10;
					ctx.font = ""+node_size+"px Tahoma,Arial,Verdana"
					ctx.textAlign = "center"
					ctx.fillStyle = "#FF4040"
					for(var i=0;i<indices.length;i++){
						roundRect(ctx,pt.x -w/2+i*20, pt.y -37 ,w_index ,20);
						ctx.fillText(indices[i], pt.x-w/2+w_index/2+i*20, pt.y-22)						
					}
				}
			},
			/*
			 * Function: draw_edge
			 * 
			 * Invoked:
			 * 	The main function to "move" an edge according to the source and destination nodes new location
			 * 	The function knows how to draw the node according to the src and dst properties
			 * 	i.e if the edge.is_cycle is on, the edge will be a round back to node edge
			 *     	    if the edge connects two selected nodes, the edge will be red too, etc
			 * 
			 * Parameters:
			 * 	edge - the edge object to redraw
			 * 	pt1 - point object of source
			 * 	pt2 - point object of dest
			 * 
			 * Returns:
			 * 	undefined
			 * 
			 * See Also:
			 * 	<init>
			 * 	<redraw>
			 * 
			 */
			draw_edge:function(edge,pt1,pt2){
				// edge: {source:Node, target:Node, length:#, data:{}}
				// pt1:  {x:#, y:#}  source position in screen coords
				// pt2:  {x:#, y:#}  target position in screen coords
				var highlight_arrow = false;
				var weight = edge.data.weight
				var color = edge.data.color
				var is_cycle=(edge.source.data.id==edge.target.data.id && edge.data.loop); 
				if (!color || (""+color).match(/^[ \t]*$/)) 
					color = null
				// find the start point
				var node_height = nodeBoxes[edge.source.name][3]
				var right_most_upper_x = nodeBoxes[edge.source.name][0]+nodeBoxes[edge.source.name][2]
				var right_most_upper_y = nodeBoxes[edge.source.name][1]
				ctx.save() 
				ctx.beginPath()
				//if it both nodes are selected, and have to consecutive indices, mark edge in red and highlight it
				if (!is_cycle && edge.source.data.selected && edge.target.data.selected && are_consecutive_nodes(edge.source,edge.target)){
					ctx.strokeStyle = (color) ? color : "#FF4040"
					ctx.lineWidth =  3
					highlight_arrow = true
				}
				//if it's a cycle, check if it has two indices that follow
				else if (is_cycle && has_consecutive_indices(edge.source)){
					ctx.strokeStyle = (color) ? color : "#FF4040"
					ctx.lineWidth =  3
					highlight_arrow = true				
				}
				else{
					ctx.lineWidth = (!isNaN(weight)) ? parseFloat(weight) : 1
					ctx.strokeStyle = (color) ? color : "#04408C"
					highlight_arrow = false
				}
				//ctx.fillStyle = null
				//if the src and dest are the same node, draw an arch
				if (is_cycle){
					var radius = 20
					ctx.beginPath();
					var borders =  nodeBoxes[edge.target.name]
					var  demo_source_box = []
					demo_source_box[0] = borders[0]+180
					demo_source_box[1] = borders[1]
					demo_source_box[2] = borders[2]
					demo_source_box[3] = borders[3]
					var pt2_demo =  arbor.Point(pt1.x+180, pt2.y)
					var tail = intersect_line_box(pt1, pt2_demo, demo_source_box)
					var head = intersect_line_box(tail, pt2, nodeBoxes[edge.target.name])
					ctx.arc(right_most_upper_x+radius/2,right_most_upper_y-3,radius,1*Math.PI-0.1,1*Math.PI-pixels_ro_radiants(node_height*3/2+10))
				}
				else{
					var tail = intersect_line_box(pt1, pt2, nodeBoxes[edge.source.name])
					var head = intersect_line_box(tail, pt2, nodeBoxes[edge.target.name])
					ctx.moveTo(tail.x, tail.y)
					ctx.lineTo(head.x, head.y)							
				}

				ctx.stroke()
				ctx.restore()

		  		// draw an arrowhead if this is a -> style edge
		  		if (edge.data.directed){
					ctx.save()
					// move to the head position of the edge we just drew
				
					if (highlight_arrow){
						ctx.fillStyle = (color) ? color : "#FF4040"
						var wt = 3
					}
					else{
						ctx.fillStyle = (color) ? color : "#04408C"
						var wt = !isNaN(weight) ? parseFloat(weight) : 1
					}
					var arrowLength = 6 + wt
					var arrowWidth = 2 + wt
					if (is_cycle){
						ctx.translate(head.x, head.y);
						ctx.rotate(Math.atan2(head.y - tail.y, head.x - tail.x));

						// delete some of the edge that's already there (so the point isn't hidden)
						ctx.clearRect(-arrowLength/2,-wt/2, arrowLength/2,wt)

						// draw the chevron
						ctx.beginPath();
						ctx.moveTo(-arrowLength, arrowWidth);
						ctx.lineTo(0, 0);
						ctx.lineTo(-arrowLength, -arrowWidth);
						ctx.lineTo(-arrowLength * 0.8, -0);
						ctx.closePath();
						ctx.fill();
						ctx.restore()						
					}
					else{
						ctx.translate(head.x, head.y);
						ctx.rotate(Math.atan2(head.y - tail.y, head.x - tail.x));

						// delete some of the edge that's already there (so the point isn't hidden)
						ctx.clearRect(-arrowLength/2,-wt/2, arrowLength/2,wt)

						// draw the chevron
						ctx.beginPath();
						ctx.moveTo(-arrowLength, arrowWidth);
						ctx.lineTo(0, 0);
						ctx.lineTo(-arrowLength, -arrowWidth);
						ctx.lineTo(-arrowLength * 0.8, -0);
						ctx.closePath();
						ctx.fill();
						ctx.restore()					
					}

		  		}			
			},
			/*
			 * Function: reset_selected_nodes
			 * 
			 * Invoked:
			 * 	Iterates over all nodes, marks them as not selected and draws them as unselected ones
			 * 
			 * Parameters:
			 * 
			 * Returns:
			 * 	undefined
			 * 
			 * 
			 */
			reset_selected_nodes:function(){
				if (!particleSystem) return;
				particleSystem.eachNode(function(node, pt){
					node.data.selected=false;
					if (node.data.indices_in_group==undefined)
						console.log(6)
					that.draw_node(node,pt,ctx,gfx)
				})
			},
			/*
			 * Function: show_node_menu
			 * 
			 * Invoked:
			 * 	When we want to show the node menu next to a certain node
			 * 
			 * Parameters:
			 * 	nearest_node - the node near which to show the menus
			 * 
			 * Returns:
			 * 	undefined
			 * 
			 * 
			 */
			show_node_menu:function(nearest_node,allow_addition_to_group){
				//reposition the menu if it's visible
				var canvas_x_offset = window.parent.Ext.getCmp('w_diagram').getPosition()[0];
				var pt = nearest_node.screenPoint;
				var node = nearest_node.node;
				node.data.menu_visible = true
				
				window.parent.node_menu_horizontal.setPosition(pt.x+canvas_x_offset, pt.y+100);

				if(!allow_addition_to_group){
					window.parent.Ext.getCmp('w_node_menu_add').hide()
					window.parent.Ext.getCmp('w_node_menu_remove').hide()		
				}
				else{
					window.parent.Ext.getCmp('w_node_menu_add').show()
					window.parent.Ext.getCmp('w_node_menu_remove').show()
				}
				//check which buttons to show on the menu
				if (node.data.expanded){
					window.parent.Ext.getCmp('w_node_menu_contract').show()
				}
				else{
					window.parent.Ext.getCmp('w_node_menu_contract').hide()							
				}
				if (node.data.expandable && !node.data.expanded){
					window.parent.Ext.getCmp('w_node_menu_expand').show()
				}
				else{
					window.parent.Ext.getCmp('w_node_menu_expand').hide()							
				}
				window.parent.node_menu_horizontal.show()
				
				
			},
			/*
			 * Function: hide_node_menu
			 * 
			 * Invoked:
			 * 	When we want to hide the node menu of to a certain node
			 * 
			 * Parameters:
			 * 	nearest_node - the node near which to hide the menus
			 * 
			 * Returns:
			 * 	undefined
			 * 
			 * 
			 */
			hide_node_menu:function(node){				
				node.data.menu_visible=false;
				window.parent.node_menu_horizontal.hide()
			},
			/*
			 * Function: debug_nodes
			 * 
			 * Invoked:
			 * 	Prints the id and label of all nodes on graph
			 * 
			 * Parameters:
			 * 
			 * Returns:
			 * 	undefined
			 * 
			 * 
			 */
			debug_nodes:function(){
				particleSystem.eachNode(function(node, pt){
					console.log("id:"+node.data.id+" label:"+node.data.label)
				})
			},
			/*
			 * Function: initMouseHandling
			 * 
			 * Invoked:
			 * 	The function runs for the first time only. Binds the canvas to all mouse events
			 * 	Sets the that.handler to be the event handler object
			 * 
			 * Parameters:
			 * 
			 * Returns:
			 * 	undefined
			 * 
			 * See Also:
			 * 	<clicked>
			 * 	<dragged>
			 * 	<dropped>
			 * 	<hovered>
			 * 	<dbl_clicked>
			 * 	<right_clicked>
			 * 	
			 */
      			initMouseHandling:function(){
				//no-nonsense drag and drop (thanks springy.js)
				selected = null;
				nearest = null;
				var dragged = null;
				var oldmass = 1
	       			//set up a handler object that will initially listen for mousedowns then
				//for moves and mouseups while dragging
				var handler = {
		  			clicked:function(e){
		  				//hide menu if its visible
		  				if (parent.w_rightclick_menu.isVisible()){
		  					parent.w_rightclick_menu.hide();
		  				}
			    			var pos = $(canvas).offset();
			    			_mouseP = arbor.Point(e.pageX-pos.left, e.pageY-pos.top)
			    			selected = nearest = dragged = particleSystem.nearest(_mouseP);
			    			if (dragged.node !== null) 
							dragged.node.fixed = true
						$(canvas).bind('mousemove', handler.dragged)
						$(window).bind('mouseup', handler.dropped)
						return false
		  			},
					dragged:function(e){
							var old_nearest = nearest && nearest.node._id
							var pos = $(canvas).offset();
							var s = arbor.Point(e.pageX-pos.left, e.pageY-pos.top)
							if (!nearest) return
							if (dragged !== null && dragged.node !== null){
								var p = particleSystem.fromScreen(s)
								dragged.node.p = p
			    				}
		    					return false
		  			},

					dropped:function(e){						
						if (dragged===null || dragged.node===undefined) 
							return
						if (dragged.node !== null) 
							dragged.node.fixed = false
						dragged.node.tempMass = 1000
						dragged = null
						selected = null
						$(canvas).unbind('mousemove', handler.dragged)
						$(window).unbind('mouseup', handler.dropped)
						_mouseP = null
						return false
		  			},
					hovered:function(e){
						var pos = $(canvas).offset();
						//get the nearest node to the mouse location
						var nearest_node=sys.nearest({x:e.pageX-pos.left,y:e.pageY-pos.top});
						//verify we found one
						if (nearest_node==null){
							return;
						}
						//check if the mouse is over a node or just on the canvas
						if (!is_over_a_node(nearest_node)){
							//if we're not on a node, hide the menu			
							if (that.node_hovered){
								that.hide_node_menu(that.node_hovered)
								that.node_hovered = undefined;
							}
							//mark and redraw all nodes as unhovered
							sys.eachNode(function(node_itr,node_point){
								node_itr.data.hovered = false;
								if (node_itr.data.indices_in_group==undefined)
									console.log(2)								
								that.draw_node(node_itr,node_point,ctx,gfx);
							})
							return;
						}
						//if we're over a node
						var node = nearest_node.node;
						//if its already hovered return
						if (that.node_hovered && node.name==that.node_hovered.name){
							return;
						}
						//if we're on edit/create group mode show add to group menu
						if (window.parent.w_pages_add_mode!=undefined){
							that.show_node_menu(nearest_node,true)
						}
						else{
							that.show_node_menu(nearest_node,false)
						}
						var pt = nearest_node.screenPoint;
						window.parent.node_selected = node;
						//make last hovered_node un-highlighted
						if (that.node_hovered!=undefined && that.node_hovered.name!=node.name){
							that.node_hovered.data.hovered = false;
							//if (that.node_hovered.data.indices_in_group==undefined)
							//	console.log(3)
							that.draw_node(that.node_hovered,pt,ctx,gfx)
						}
						
						//make the new node the global hovered one and re-draw it as highlighted
						node.data.hovered=true;
						that.node_hovered = node;
						that.draw_node(node,pt,ctx,gfx)
						if (!that.node_hovered)
							window.parent.hide_tooltip()
						//window.parent.change_graph_definition()
					},
					dbl_clicked:function(e){
						//on dbl click, if we're on edit/create group mode, add it to the group
						if (that.node_hovered!=undefined && window.parent.w_pages_add_mode!=undefined){
							add_node_to_group()
						}					
					},
					right_clicked:function(e){
						e.preventDefault();
			    			var pos = $(canvas).offset();
						//show right click menu only for page nodes
						if (!that.node_hovered || (that.node_hovered.data.type!='page'))
							return;
						that.node_rightclicked = that.node_hovered;
						var nearest_node=sys.nearest({x:e.pageX-pos.left,y:e.pageY-pos.top});
						var pt = nearest_node.screenPoint;
						var node = nearest_node.node;				
						that.handler.dropped();
						//position right click menu over clicked node
						
						if (node.data.has_alias && parseInt(node.data.has_alias)){
							window.parent.Ext.getCmp('w_rightclick_menu4').show();
							window.parent.Ext.getCmp('w_rightclick_menu1').setText("Edit alias");
						}
						else{
							window.parent.Ext.getCmp('w_rightclick_menu4').hide();
							window.parent.Ext.getCmp('w_rightclick_menu1').setText("Add alias");
						}

						parent.w_rightclick_menu.showAt(pt.x+canvas_x_offset-node.data.width*0.5,pt.y+55);
						return false;
					}
				}
				$(canvas).mousedown(handler.clicked);
				$(canvas).bind('mousemove',handler.hovered);
				$(canvas).bind('dblclick',handler.dbl_clicked);
				$(canvas).bind('contextmenu',handler.right_clicked);
				/*window.parent.Ext.getCmp('w_node_menu_add').addListener(
					'click',
					function(){
						add_node_to_group()
					}
				)
				window.parent.Ext.getCmp('w_node_menu_remove').addListener(
					'click',
					function(){
						remove_node_from_group()
					}
				)*/
				that.handler = handler
	      		}
			
			
    		}//end of that object

		function add_node_to_group(){
			window.parent.add_node_to_group(that.node_hovered)						
		}
		function remove_node_from_group(){	
			window.parent.remove_node_from_table(that.node_hovered.data.full_label)
		}
		/*
		 * Function: clean_node
		 * 
		 * Invoked:
		 * 	Erases the node from the canvas
		 * 
		 * Parameters:
		 * 	node - the node to erase
		 * 	pt - the nodes location by point
		 * 	w - the nodes width
		 * 	ctx - the canvas context
		 * 
		 * Returns:
		 * 	undefined
		 * 
		 * See Also:
		 * 	<draw_node>
		 * 	
		 */
		function clean_node(node,pt,w,ctx){
			//erase previous lines
			ctx.lineWidth = 5
			ctx.strokeStyle = "#FFFFFF"
			roundRect(ctx,pt.x -w/2, pt.y -16 , w,30);
			ctx.clearRect(pt.x -w/2, pt.y -16 , w,30)
			if (node.selected){
				ctx.clearRect(pt.x -w/2, pt.y -16 , w,30)				
			}
			//remove text(may be redundant)
  			if (node.data.label){
  				var node_size = 3*2+10+3;
				ctx.font = " "+node_size+"px Tahoma,Arial,Verdana"
				ctx.textAlign = "center"
				ctx.fillStyle = "#FFFFFF"
				ctx.fillText(node.data.label, pt.x, pt.y+5)
  			}
		}

		// helpers for figuring out where to draw arrows 
		var intersect_line_line = function(p1, p2, p3, p4)
		{
			var denom = ((p4.y - p3.y)*(p2.x - p1.x) - (p4.x - p3.x)*(p2.y - p1.y));
			if (denom === 0) 
				return false // lines are parallel
			var ua = ((p4.x - p3.x)*(p1.y - p3.y) - (p4.y - p3.y)*(p1.x - p3.x)) / denom;
			var ub = ((p2.x - p1.x)*(p1.y - p3.y) - (p2.y - p1.y)*(p1.x - p3.x)) / denom;

			if (ua < 0 || ua > 1 || ub < 0 || ub > 1)  
				return false
			return arbor.Point(p1.x + ua * (p2.x - p1.x), p1.y + ua * (p2.y - p1.y));
		}

		var intersect_line_box = function(p1, p2, boxTuple)
		{
			var p3 = {x:boxTuple[0], y:boxTuple[1]},
			    w = boxTuple[2],
	          	    h = boxTuple[3]
			var tl = {x: p3.x, y: p3.y};
			var tr = {x: p3.x + w, y: p3.y};
			var bl = {x: p3.x, y: p3.y + h};
			var br = {x: p3.x + w, y: p3.y + h};

      			return 	intersect_line_line(p1, p2, tl, tr) ||
            			intersect_line_line(p1, p2, tr, br) ||
            			intersect_line_line(p1, p2, br, bl) ||
            			intersect_line_line(p1, p2, bl, tl) ||
            			false
    		}
		/*
		 * Function: roundRect
		 * 
		 * Invoked:
		 * 	Erases the node from the canvas
		 * 
		 * Parameters:
		 * 	ctx - the canvas context
		 * 	x - x location of the rect
		 * 	y - y location of the rect
		 * 	width - the nodes width
		 * 	height - the nodes height
		 * 	
		 * 	
		 * 
		 * Returns:
		 * 	undefined
		 * 
		 * See Also:
		 * 	<draw_node>
		 * 	
		 */
		function roundRect(ctx, x, y, width, height, radius, fill, stroke) {
			  if (typeof stroke == "undefined" ) {
			    stroke = true;
			  }
			  if (typeof radius === "undefined") {
			    radius = 5;
			  }
			  ctx.beginPath();
			  ctx.moveTo(x + radius, y);
			  ctx.lineTo(x + width - radius, y);
			  ctx.quadraticCurveTo(x + width, y, x + width, y + radius);
			  ctx.lineTo(x + width, y + height - radius);
			  ctx.quadraticCurveTo(x + width, y + height, x + width - radius, y + height);
			  ctx.lineTo(x + radius, y + height);
			  ctx.quadraticCurveTo(x, y + height, x, y + height - radius);
			  ctx.lineTo(x, y + radius);
			  ctx.quadraticCurveTo(x, y, x + radius, y);
			  ctx.closePath();
			  if (stroke) {
			    ctx.stroke();
			  }
			 // if (fill) {
			  //  ctx.fill();
			  //}        
		}
		/*
		 * Function: is_over_a_node
		 * 
		 * Invoked:
		 * 	Checks if the mouse is over a node
		 * 
		 * Parameters:
		 * 	nearest_node - the nearest node object to the mouse
		 * 	event - the event object of the hover event
		 * 	
		 * 
		 * Returns:
		 * 	true/false - true if mouse is over the node. false else wise
		 * 
		 * See Also:
		 * 	<draw_node>
		 * 	
		 */
		function is_over_a_node(nearest_node,event){
			var node = nearest_node.node
			var dist = nearest_node.distance
			//take dist from center of the node to its corner(Pythagoras) if not already calculated
			if (node.data.diagonal==undefined || isNaN(node.data.diagonal)){
				var h = 30//height of node
				var w = node.data.width/2
				var diagonal = Math.sqrt(Math.pow(30,2)+Math.pow(w,2))
				node.data.diagonal = diagonal+20//for the buttons
			}
			return (dist<=node.data.diagonal)
			
		}
		/*
		 * Function: are_consecutive_nodes
		 * 
		 * Invoked:
		 * 	checks if source and destination nodes are consecutive in a workflow group to know if
		 * 	to draw an edge between them
		 * 
		 * Parameters:
		 * 	source - the source node object
		 * 	target - the target node object
		 * 	
		 * 
		 * Returns:
		 * 	true/false 
		 * 
		 * See Also:
		 * 	<draw_node>
		 * 	
		 */
		function are_consecutive_nodes(source,target){
			var from_inds = source.data.indices_in_group
			var to_inds = target.data.indices_in_group
			for (var i = from_inds[from_inds.length-1];i>=0;i--){
				for (var j = to_inds[to_inds.length-1];j>=0;j--){
					if (to_inds[j]<=from_inds[i])
						break;
					if (to_inds[j]==(from_inds[i]+1))
						return true;
				}				
			}
			return false;
		}
		/*
		 * Function: has_consecutive_indices
		 * 
		 * Invoked:
		 * 	checks if source and destination nodes have any consecutive indices in their property to know if
		 * 	to draw an edge between them
		 * 
		 * Parameters:
		 * 	source - the source node object
		 * 	target - the target node object
		 * 	
		 * 
		 * Returns:
		 * 	true/false 
		 * 
		 * See Also:
		 * 	<draw_node>
		 * 	
		 */
		function has_consecutive_indices(node){
			var indices = node.data.indices_in_group
			for (var i=0;i<indices.length-1;i++){
				if (indices[i]==indices[i+1]-1)
					return true
			}
			return false
		}
		function pixels_ro_radiants(px){
			var p = parseFloat(px); 
			var r = Math.atan(p / 5376) * 2; 
			var d = r * 180 / Math.PI; 
			d = d.toFixed(3); 			
			return parseFloat(d)
		}

    		return that
  	}    

  
})()
