	trace = arbor.etc.trace
	objmerge = arbor.etc.objmerge
	objcopy = arbor.etc.objcopy
	//var parse = Parseur().parse

  	var HalfViz = function(elt) {
	
	    var dom = $(elt);
		var node_repulsion = 15000;
		var rigidity = 50;
		var friction = 0.25;
		//sys = arbor.ParticleSystem(2600, 512, 0.5)
		sys = arbor.ParticleSystem(node_repulsion,rigidity,friction);
		sys.renderer = Renderer("#viewport") // our newly created renderer will have its .init() method called shortly by sys...
		parent.window.sys = sys;
//		sys.screenPadding(50);
		sys.screen({ padding:[100, 100, 100, 100], step:1 });
		sys.parameters({ gravity: true });
	    
		//var _ed = dom.find('#editor')
		//var _code = dom.find('textarea')
		var _canvas = dom.find('#viewport').get(0)
		//var _grabber = dom.find('#grabber')

		var _updateTimeout = null
		var _current = null // will be the id of the doc if it's been saved before
		var _editing = false // whether to undim the Save menu and prevent navigating away
		var _failures = null
	    
	    	var that = {
	      		init:function(){
				window.parent.Ext.getCmp('w_diagram').addListener('resize',function(){
					that.resize()
				})
				$(window).resize(that.resize)
				that.resize()
				that.updateLayout(Math.max(1, $(window).width()));
				return that
	      		},
	      

	      
	      		resize:function(){        
				var w = $(window).width();
				var x = w//_ed.width()
				that.updateLayout(x)
				sys.renderer.redraw()
	      		},
	      
	      		updateLayout:function(split){
				var w = dom.width();
				var h = dom.height() - 100;//_grabber.height()
				var split = split //|| _grabber.offset().left

				var canvW = w
				var canvH = h
				_canvas.width = canvW
				_canvas.height = canvH
				sys.screenSize(canvW, canvH)
				
	      		},
	      
			grabbed:function(e){
				$(window).bind('mousemove', that.dragged)
				$(window).bind('mouseup', that.released)
				return false
			},
			dragged:function(e){
				var w = dom.width()
				that.updateLayout(Math.max(10, Math.min(e.pageX-10, w)) )
				sys.renderer.redraw()
				return false
			},
			released:function(e){
				$(window).unbind('mousemove', that.dragged)
				return false
			}
		}
    
    		return that.init()    
  	}