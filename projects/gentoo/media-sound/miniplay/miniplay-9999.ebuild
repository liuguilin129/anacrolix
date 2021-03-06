# Copyright 1999-2008 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

#EAPI=1
inherit eutils subversion
DESCRIPTION="A parsimonious audio player in GTK+."

HOMEPAGE="http://eruanno.net/"

#SRC_URI="ftp://foo.bar.com/${P}.tar.gz"
ESVN_REPO_URI="https://stupidape.dyndns.org/svn/miniplay/trunk"

LICENSE=""

SLOT="0"

KEYWORDS="~x86 ~amd64"

#RESTRICT="strip"

DEPEND="
	media-libs/gstreamer
	media-libs/gst-plugins-base
	media-plugins/gst-plugins-meta
	x11-libs/gtk+:2
	x11-libs/libnotify
"

RDEPEND="${DEPEND}"

#S="${WORKDIR}/${P}"

src_compile() {
	autoreconf --install --force
	econf || die "econf failed"
	emake || die "emake failed"
}

src_install() {
	emake DESTDIR="${D}" install || die "emake install failed"
}
