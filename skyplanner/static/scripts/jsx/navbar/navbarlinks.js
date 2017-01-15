class NavbarLinks extends React.Component {
    constructor(props) {
        super(props);
    }
    render() {
        var classname = 'nav navbar-nav';
        if(this.props.classname !== undefined)
            classname += ' ' + this.props.classname;
        return <ul className={classname}>{this.props.children}</ul>;
    }
}
export default NavbarLinks;
