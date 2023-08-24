$(function() {
  const refresh = function() {
    $.ajax('/api/variable/song-title', {
      type: 'get',
      dataType: 'text'
    }).then(function(data) {
      data.trim();
      if (data.length == 0) {
        $("#song_title").hide();
      }
      else {
        $("#song_title").show();
      }
      $("#song_title").text(data);
    });
    $.ajax('/api/variable/song-list', {
      type: 'get',
      dataType: 'text'
    }).then(function(data) {
      $("#song_list").html(data.replaceAll('\n','<br/>'));
    });
  };
  refresh();
  setInterval(refresh, 1000);
})