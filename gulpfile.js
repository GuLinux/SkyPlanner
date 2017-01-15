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
    broserify(
            {
                entries: ['./skyplanner/static/scripts/jsx/main.js'],
                paths: ['./skyplanner/static/scripts/jsx']
            })
        .transform(babelify, {presets: ["es2015", "react"]})
        .bundle()
        .pipe(fs.createWriteStream("./skyplanner/static/scripts/js/bundle.js"))
        ;
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
