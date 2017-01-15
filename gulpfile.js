// requirements

var gulp = require('gulp');
var broserify = require('browserify');
var babelify = require("babelify");

var del = require('del');
var fs = require('fs');

// tasks

gulp.task('transform', function () {
    try {
        fs.mkdirSync('./skyplanner/static/scripts/js');
    }
    catch(e) {
    }
    broserify('./skyplanner/static/scripts/jsx/main.js')
        .transform(babelify, {presets: ["es2015", "react"]})
        .bundle()
        .pipe(fs.createWriteStream("./skyplanner/static/scripts/js/bundle.js"))
        ;
});

gulp.task('del', function () {
    return del(['./project/static/scripts/js']);
});

gulp.task('default', function() {
    gulp.start('transform');
});
