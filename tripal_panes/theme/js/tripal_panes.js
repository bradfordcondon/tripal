(function($) {

  Drupal.behaviors.tripal_panes = {
    attach: function (context, settings){
    
      // Add a close button for each pane
      $('.tripal_pane-fieldset .fieldset-legend').each(function (i) {
        $(this).append('<div class="tripal_pane-fieldset-close_button">[X]</div>');
      });
      
      // Hide the pane when the close button is clicked
      $('.tripal_pane-fieldset-close_button').each(function (i) {
        $(this).css('float', 'right');
        $(this).css('cursor', 'pointer');
        $(this).css('margin', '0px 5px');
        $(this).click(function () {
          var fs = $(this).parent().parent().parent();
          var fsid = fs.attr('id');
          if (fsid.indexOf('tripal_pane-fieldset-') == 0) {
            $(fs).fadeOut(300);
          }
        });
      });
      
      // Move the pane to the first when its TOC item is clicked.
      $('.tripal_toc_list_item_link').each(function (i) {
        $(this).click(function() {
          var id = '#tripal_pane-fieldset-' + $(this).attr('id');
          $(id).removeClass('collapsed');
          $(id + ' .fieldset-wrapper').show();
          var prevObj = $(id).prev().attr('class');
          
          // Highlight the pane if it's already at the top
          if (prevObj.indexOf('tripal_pane-base_pane') == 0 && $(id).css('display') == 'block') {
            var color = $(id).css('background-color') ? $(id).css('background-color') : '#FFFFFF';
            if (jQuery.ui) {
              $(id).fadeTo(10, 0.5, function() {});
              $(id).fadeTo(100, 1, function() {});
              //$(id).effect('highlight', {color: '#DDDEEE'});
            }
          }
          // Move the pane
          else {
            $(id).hide();
            var obj = $(id).detach();
            $('.tripal_pane-base_pane').after(obj);
            $(id).show(300);
          }
          return false;
        });
      });
    },
  };
  
})(jQuery);
