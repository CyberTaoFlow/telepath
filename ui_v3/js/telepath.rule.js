telepath.ruleOverlay = {
	
	addRule: function () {
		
		var that = this;
		var rule_id = 1;
				
		telepath.ds.get('/rules', { mode: 'expand_group', id: rule_id }, function(data) {
			that.data = data;
			// console.log(data);
			that.initUI({ id: 'new', name: 'New Rule', data: data });
		});
		
	},
	initUI: function (data) {
		
		// Check conversion
		if(typeof(data.data) == 'string') {
			data.data = JSON.parse(data.data);
		}

		var that = this;
		this.data = data;
		
		// Show Window
		telepath.overlay.init('rule-edit', data.name);
		$('.tele-overlay').height(800).trigger('resize');
		
		// Rule Name
		var ruleName = $('<div>').teleInput({ label: 'Name', value: data.id == 'new' ? '' : data.name });
		telepath.overlay.contentEl.append(ruleName);
		
		// Rule Owner
		/*var ruleOwner = $('<div>').teleInput({ label: 'Owner', value: data.id == '' ? '' : data.owner });
		telepath.overlay.contentEl.append(ruleOwner);*/
		
		// Rule Desc
		var ruleDesc = $('<div>').teleInput({ label: 'Description', value: data.id == '' ? '' : data.desc });
		telepath.overlay.contentEl.append(ruleDesc);
		
		var ruleScore = $('<div>').teleInput({ label: 'Score', width: 30, value: data.score == '' ? '95' : data.score });
		telepath.overlay.contentEl.append(ruleScore);
		
		// Title
		var title = $('<div>').addClass('tele-title-1').text('Select type and build rule');
		telepath.overlay.contentEl.append(title);
		
		// Condition list
		var cond = $('<div>').teleRule({ data: data.data });
		telepath.overlay.contentEl.append(cond);
	
		// Apply / Cancel buttons
		
		var btnContain = $('<div>').addClass('tele-button-container');
		var saveBtn   = $('<a href="#" class="tele-button tele-button-apply">Save</a>');
		var cancelBtn  = $('<a href="#" class="tele-button tele-button-cancel">Cancel</a>');
		
		btnContain.append(saveBtn).append(cancelBtn);
		
		telepath.overlay.contentEl.append(btnContain);
		
		// Callbacks
		saveBtn.click(function () {
			
			var json = cond.data('tele-conditionList').getJSON();
			var name = caseName.data('tele-teleInput').input.val();
			
			caseName.removeClass('error');
			if(name.length == 0 || name.length > 32) {
				caseName.addClass('error');
				return;
			}
			if(json.length == 0) {
				telepath.dialog({ title: 'Case Editor', msg: 'Must have at least one condition' });
				return;
			}

			$('.tele-icon-rule-edit').append(telepath.loader).css({ backgroundColor: 'white' });

		});
		
		cancelBtn.click(function () {
		
			telepath.overlay.destroy();
			
		});
		
	}

}
