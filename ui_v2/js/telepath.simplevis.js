var colors = d3.scale.category10();
SimpleBubble = function(data, container) {

  this.data = data;
  this.id   = data.id;
  this.canvas = container;
  this.el = null;
  this.x = 0;
  this.y = 0;
  this.radius = 0;
  this.boxSize = 0;
  this.isDragging = false;
  this.isSelected = false;
  this.tooltip = null;

  this.init();
  
};

SimpleBubble.prototype.init = function() {

	var node = this.data;

	/* Elements that make up the bubbles display*/
	this.el = $("<div class='bubble node' id='bubble-" + this.id + "'></div>");
	this.elFill = $("<div class='bubbleFill bg'></div>");
	this.el.append(this.elFill);
	
	/* Attach mouse interaction to root element */
	/* Note use of $.proxy to maintain context */
	this.el.on('mouseover', $.proxy(this.showToolTip, this));
	this.el.on('mouseout',  $.proxy(this.hideToolTip, this));
	this.el.on('click',     $.proxy(this.expand, this));

	// Add an icon
	// var icon = $('<span>').addClass('node-' + node.type);

	// Add a label
	var label     = $('<span>').addClass('label');
	label.text(node.label);
	this.el.append(label);

	// Radius
	var percent = ((node.frequency - min_frequency) / max_frequency) * 100;
	this.el.css('borderColor', colorPercent(percent, myColours));
	this.el.css('padding', percent / 4);
	this.el.css('border-radius' , 10);
	
	// D3 Props
	this.radius = 300;//this.el.width() + 100; //percent / 2;
	this.boxSize = 300;//this.el.width() + 100; //percent;
	
	// Setup CSS
	/*this.elFill.css({
		width: this.boxSize,
		height: this.boxSize,
		left: -this.boxSize / 2,
		top: -this.boxSize / 2,
		"background-color": colors(this.data)
	});*/
	
};

SimpleBubble.prototype.expand = function () {
	
	if(node.expandable) {
			
		var expandable = $('<span>').addClass('expand');
		nodeDOM.append(expandable);
		expandable.click(function () {
			
			if($(this).hasClass('open')) {
			
				// POP Current
				last_expanded.pop();
				// POP Previous
				var previous = last_expanded.pop();
				
				if(previous.id == 'application') {
					update_graph_by_app();
				} else {
					that.expand(previous);
				}
				
			} else {
				that.expand($(this).parent().data('node'));
			}
			
		});
		
		if(last_expanded[last_expanded.length - 1].id == node.id) {
			expandable.addClass('open');
		}
				
	}
	
}

SimpleBubble.prototype.showToolTip = function() {
  var toolWidth = 40;
  var toolHeight = 25;
  this.tooltip =  $("<div class='tooltip'></div>");
  this.tooltip.html("<div class='tooltipFill'><p>" + this.data + "</p></div>");
  this.tooltip.css({
    left: this.x + this.radius /2,
    top: this.y + this.radius / 2
    });
  this.canvas.append(this.tooltip);
};

SimpleBubble.prototype.hideToolTip = function() {
  $(".tooltip").remove();
};

SimpleBubble.prototype.move = function() {
  this.el.css({top: this.y, left:this.x});
  //jsPlumb.repaint(this.el);
};