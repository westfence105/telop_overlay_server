$(function() {
  const fetch = function() {
    $.ajax('/api/variable/song-title', {
      type: 'get',
      dataType: 'text'
    }).then(
      function (value) {
        $('#song_title').val(value);
      }
    );
    $.ajax('/api/variable/song-list', {
      type: 'get',
      dataType: 'text'
    }).then(
      function (value) {
        $('#song_list').val(value);
      }
    );
  };
  fetch();

  $("#send_title").on('click', function() {
    $.ajax('/api/variable/song-title', {
      type: 'get',
      dataType: 'text'
    }).then(
      function (data) {
        const title = $("#song_title").val();
        $.ajax('/api/variable/song-title', {
          type: 'post',
          data: title,
          dataType: 'text'
        }).then(fetch);

        var song_list = $('#song_list').val();
        if (song_list.length > 0) {
          song_list = song_list.concat("\n", title);
        }
        else {
          song_list = title;
        }
        $('#song_list').val(song_list);
        $.ajax('/api/variable/song-list', {
          type: 'post',
          data: song_list,
          dataType: 'text'
        }).then(fetch);
      }
    );
  });

  $("#send_list").on('click', function() {
    const song_list = $('#song_list').val();
    $.ajax('/api/variable/song-list', {
      type: 'post',
      data: song_list,
      dataType: 'text'
    }).then(
      function(data){
        fetch();
      }
    );
  });
})