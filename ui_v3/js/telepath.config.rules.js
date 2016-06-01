telepath.config.rules = {
	searchString:'',

	init:function(){

		var that= this;

		//this.reload();

		this.barLeft.empty();

		this.cmdNewRule     = $('<div>').btn({ icon: 'plus', text: 'New Rule', callback: function () {
			if (telepath.access.admin || telepath.access.perm.Rules_add){
				telepath.config.rule.editRule('new');
				//telepath.ruleOverlay.addRule();
			}else{
				telepath.dialog({msg:'Access denied. No permissions to create a new Rule.'});
			};
		} }).hide();

		this.cmdNewCategory = $('<div>').btn({ icon: 'plus', text: 'New Category' , callback: function () {
			if (telepath.access.admin || telepath.access.perm.Rules_add){
				telepath.config.rules.addCategory();
			}else{
				telepath.dialog({msg:'Access denied. No permissions to create a new Category.'});
			};
		} });



		this.barLeft/*.append(leftTitle)*/.append(this.cmdNewRule).append(this.cmdNewCategory) ;

		this.barRight.empty();

		var rightTitle = $('<div>').addClass('tele-panel-subtitle-text').html('Editor');

		this.search = $('<div>').teleSearch({ callback: function (e, txt) {
			that.searchString = txt;
			//that.init();
		}});
		this.barRight.append(rightTitle).append(this.search);

		$("#search-button").on("click", function (event) {
			that.searchString = '';
			$(".tele-config-bar-right .tele-search-input").prop("value", that.searchString);
			that.input();

		});


		if (typeof(that.searchString) != 'undefined'){
		 $(".tele-config-bar-right .tele-search-input").prop("value", that.searchString);
		 that.input();
		 }


		// add search on client site on key up event
		$(".tele-config-bar-right .tele-search-input").keyup('input', function () {
			that.searchString = $(this).val();
			// console.log(that.searchString);
			that.input();
		});
	},

	reload: function () {
		
		var that = this;
		
		this.contentLeft.empty();
		this.contentRight.empty();
		that.cmdNewRule.hide();


		//this.input();

		// Left Side
		var leftTitle = $('<div>').addClass('tele-panel-subtitle-text').html('Rules');
		this.ruleTree = $('<div>').teleTree({ 
			type: 'rules', 
			callback: function(e, data) {
				// console.log(data);

				if(data.node.data.type == 'category') {
					that.cmdNewRule.show();
					that.selectedCategory = data.node.data.name;
				} else {
					that.cmdNewRule.hide();
				}

				if(data.node.data.type == 'group') {
					telepath.config.rule.editRule(data.node.data.name, data.node.data.category);
				}
			}
		}).css({ height: this.contentLeft.css('height') });

		//this.contentLeftWrap = $('<div>');
		//this.contentLeft.empty().append(this.contentLeftWrap);
		//this.contentLeftWrap.append(this.ruleTree);
		
		this.contentLeft.append(this.ruleTree);
		


	},
	input: function(){
		var that = this;
		var icon= $("#search-button");
		if (that.searchString.length>0)
			icon.addClass('icon-delete-input2').removeClass("tele-search-button");
		else
			icon.removeClass('icon-delete-input2').addClass("tele-search-button");

		that.reload();
		that.ruleTree.teleTree('option', 'searchString', that.searchString);


	},
	delCategory: function(value) {
	
		telepath.dialog({type:'dialog',
			msg:'This operation will delete the selected rule category. Are you sure?', 
			callback:function(){
				
				telepath.ds.get('/rules/del_category', { cat: value }, function(data) {
					telepath.config.rules.init();
				});
								
			}});

	},
	addCategory: function (msg, error) {
		
		if(!error) {
			error = false;
		}
		
		if(!msg) {
			msg = 'New category name:'
		}
		
		telepath.dialog({ type: 'prompt', title: 'Create new category', error: false, msg: msg, callback: function (value) {
		
			var found = false;
			$.each(telepath.config.rules.categories, function(i, v) {
				 // console.log(value);
				 // console.log(v.name);
				 if(v.name.toLowerCase() == value.toLowerCase()){
				 	found = true;
				 };
			});

			if(found) {
				telepath.config.rules.addCategory('Choose a different name:', true);
				return;
			}
			
			telepath.ds.get('/rules/add_category', { cat: value }, function(data) {
				telepath.config.rules.init();
			});

		}
	});
	}
}