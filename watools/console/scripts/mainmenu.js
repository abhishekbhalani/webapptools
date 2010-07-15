var prevButton = "";
var prevIndex = -1;
var methods = Array('tasks.php','profiles.php','reports.php','dicts.php','vulners.php','settings.php');
var lastMethod = 6;

$(function() {
	$("#main-menu").selectable({
			stop: function(){
				var sz = $(".ui-selected", this).size();
				var newPrev = -1;
				$(".ui-selected", this).each(function(){
					var index = $("#main-menu li").index(this);
					if (index == prevIndex) {
						if (sz > 1) {
							$(this).toggleClass("ui-selected");
						}
					}
					else {
						newPrev = index;
						if (newPrev == lastMethod) {
							document.location = "/logout.php";
							return;
						}
						$("#mainView").load(methods[newPrev]);
					}
				});
				prevIndex = newPrev;
			}
		});
});
