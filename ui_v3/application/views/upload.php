<form id="fileupload" action="" method="POST" enctype="multipart/form-data">

	<!-- The fileupload-buttonbar contains buttons to add/delete files and start/cancel the upload -->
	<div class="fileupload-buttonbar">
			<!-- The fileinput-button span is used to style the file input field as button -->
			<span class="fileinput-button">
				<input type="file" name="files[]" multiple />
			</span>
			<button type="submit" class="start">
				<span>Start upload</span>
			</button>
			<button type="reset" class="cancel">
				<span>Cancel upload</span>
			</button>
	</div>
	<!-- The table listing the files available for upload/download -->
	<div class="fileupload-dragcontainer">
		Drag and Drop your files here
	</div>
	<div class="table-uploader-container">
		<table role="presentation" class="table-uploader table-striped"><tbody class="files"></tbody></table>
	</div>
		
</form>

<!-- The template to display files available for upload -->
<script id="template-upload" type="text/x-tmpl">
{% for (var i=0, file; file=o.files[i]; i++) { %}
    <tr class="template-upload fade">
        <td>
            <p class="name">{%=file.name%}</p>
            {% if (file.error) { %}
                <div><span class="label label-danger">Error</span> {%=file.error%}</div>
            {% } %}
        </td>
        <td>
            <p class="size">{%=o.formatFileSize(file.size)%}</p>
            {% if (!o.files.error) { %}
                <div class="progress progress-striped active" role="progressbar" aria-valuemin="0" aria-valuemax="100" aria-valuenow="0"><div class="progress-bar progress-bar-success" style="width:0%;"></div></div>
            {% } %}
        </td>
        <td>
            {% if (!o.files.error && !i && !o.options.autoUpload) { %}
                <button class="btn btn-primary start">
                    <i class="glyphicon glyphicon-upload"></i>
                    <span>Start</span>
                </button>
            {% } %}
            {% if (!i) { %}
                <button class="btn btn-warning cancel">
                    <i class="glyphicon glyphicon-ban-circle"></i>
                    <span>Cancel</span>
                </button>
            {% } %}
        </td>
    </tr>
{% } %}
</script>
<!-- The template to display files available for download -->
<script id="template-download" type="text/x-tmpl">
{% for (var i=0, file; file=o.files[i]; i++) { %}
    <tr class="template-download fade">
        <td>
            <p class="name">
                <span>{%=file.name%}</span>
            </p>
            {% if (file.error) { %}
                <div><span class="label label-danger">Error</span> {%=file.error%}</div>
            {% } %}
        </td>
        <td>
            <span class="size">{%=o.formatFileSize(file.size)%}</span>
        </td>
        <td>
            Done!
        </td>
    </tr>
{% } %}
</script>