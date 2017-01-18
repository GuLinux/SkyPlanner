// requirements

var gulp = require('gulp');
var del = require('del');
var fs = require('fs');
var webpack = require('webpack-stream');

// tasks

gulp.task('transform', function () {
    try {
        fs.mkdirSync('./skyplanner/static/scripts/js');
    }
    catch(e) {
    }
    var src = './skyplanner/static/scripts/jsx/main.js';
    var dest = './skyplanner/static/scripts/js';
    return gulp.src(src)
        .pipe(webpack(require('./webpack.config.js')))
        .pipe(gulp.dest(dest));
});

gulp.task('del', function () {
    return del(['./skyplanner/static/scripts/js']);
});

gulp.task('watch', ['del'], function() {
    try {
        gulp.start('transform');
        gulp.watch('./skyplanner/static/scripts/jsx/**/*.js', ['transform']);
    }
    catch(e) {
        console.log(e);
    }
});

gulp.task('default', ['del'], function() {
    gulp.start('transform');
});
