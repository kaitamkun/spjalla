#!/usr/bin/env node
const fs = require("fs"),
      path = require("path"),
      minimist = require("minimist");

const yikes = (...a) => { console.error(...a); process.exit(1); };

const [,, ...args] = process.argv;

const options = minimist(process.argv.slice(2), {
	alias: {
		n: "namespace",
		s: "nosrc",
		i: "inherit",
		k: "keyword",
		c: "noclass",
		v: "visibility",
		p: "preview",
		m: "makevar",
		h: "noheader",
		b: "base",
		u: "usebase",
		f: "filename"
	},
	boolean: ["nosrc", "noclass", "preview", "noheader", "usebase"],
	default: {
		nosrc: false,
		noclass: false,
		preview: false,
		noheader: false,
		usebase: false,
		keyword: "class",
		visibility: "public",
		makevar: "COMMONSRC",
		base: "spjalla",
	}
});

let [name, type] = options._;
if (!type) type = "auto";
let {namespace, nosrc, inherit, keyword, noclass, visibility, preview, makevar, noheader, base, usebase, filename} = options;

const isBad = () => {
	if (!name) return true;
	for (const obj of [namespace, nosrc, inherit, keyword, noclass, visibility, preview, makevar, noheader, base, usebase, filename])
		if (obj instanceof Array) return true;
	return false;
};

if (isBad()) {
	yikes("Usage: new.js <name> [type] [-n/--namespace] [-s/--nosrc] [-h/--noheader] [-c/--noclass] [-i/--inherit superclass] [-k/--keyword (struct|class)] [-v/--visibility inheritance visibility] [-b/--base base namespace] [-u/--usebase] [-f/--filename alt. filename]");
}

if (!name.match(/^[\w_\d]+$/i)) {
	yikes("Invalid name:", name);
}

let allDir, sourcename, headername, sourcetext, headertext;

const sourcebase = "src", headerbase = "include";
let sourcedirs = [], headerdirs = [];


let fullNamespace;
if (!namespace) {
	fullNamespace = namespace = base;
} else if (namespace.indexOf(base + "::") == 0) {
	const shorter = namespace.substr(base.length + 2);
	fullNamespace = usebase? namespace : shorter;
	namespace = shorter;
} else if (usebase) {
	fullNamespace = base + "::" + namespace;
} else {
	fullNamespace = namespace;
}

console.log({base, usebase, namespace, fullNamespace});

const setDirs = (...a) => { sourcedirs = [...a]; headerdirs = [...a]; };
const setNames = s => { sourcename = `${filename || s}.cpp`; headername = `${filename || s}.h`; };
const upper = () => name.toUpperCase();
const sourceDir = () => `${sourcebase}/${sourcedirs.join("/")}`;
const headerDir = () => `${headerbase}/${headerdirs.join("/")}`;
const sourcePath = () => `${sourceDir()}/${sourcename}`;
const headerPath = () => `${headerDir()}/${headername}`;
const headerIncl = () => `${headerdirs.join("/")}/${headername}`;

if (type == "core") {
	setDirs("core");
	setNames(name);

	headertext = `
#ifndef CORE_${upper()}_H_
#define CORE_${upper()}_H_

namespace ${fullNamespace} {
	
}

#endif
`.substr(1);

	sourcetext = `
#include "core/${name}.h"

namespace ${fullNamespace} {

}
`.substr(1);

	updateModule();
} else if (type == "auto") {
	if (!namespace) yikes("Namespace not given");
	const spl = namespace.split(/::/);
	setNames(name);
	setDirs(...spl);
	console.log("Source path:", sourcePath());
	console.log("Header path:", headerPath());
	console.log("Header dir:", headerIncl());

	let classDef = `${keyword} ${name}`;
	if (inherit && inherit !== true) {
		classDef += ": ";
		if (visibility != "none") classDef += visibility + " ";
		classDef += inherit;
	}
	classDef += " {\n\t\t\n\t};";

	sourcetext = `
#include "${headerIncl()}"

namespace ${fullNamespace} {
	
}
`.substr(1);

	const guard = [...headerdirs, filename || name].join("_").toUpperCase() + "_H_";

	headertext = `
#ifndef ${guard}
#define ${guard}

namespace ${fullNamespace} {
	${noclass? "" : classDef}
}

#endif
`.substr(1);

	updateModule();
} else {
	console.error("Unknown type:", type);
	yikes(`Expected "auto" | "core"`);
}

if (sourcetext && !nosrc)    write(sourcePath(), sourcetext);
if (headertext && !noheader) write(headerPath(), headertext);

if (allDir) {
	const allText = read(`${headerbase}/${allDir}/all.h`, true);
	if (allText.indexOf(`#include "${name}.h"`) == -1) {
		write(`${headerbase}/${allDir}/all.h`, allText.replace(/(\n#endif)/, `#include "${name}.h"\n$1`));
	}
}

function updateModule() {
	let moduleText = read(`${sourceDir()}/module.mk`, true);
	if (moduleText.indexOf(sourcePath()) == -1) {
		write(`${sourceDir()}/module.mk`, moduleText.replace(/\n*\s*$/, "\n") + `${makevar} += ${sourcePath()}\n`);
	}
}

function read(where, safe=false) {
	if (safe) {
		try {
			return read(where, false);
		} catch(e) {
			if (typeof safe == "string") {
				return `\x1b[2;31m${safe}\x1b[0m`;
			} else {
				return "";
			}
		}
	} else {
		return fs.readFileSync(where, "utf8");
	}
}

function write(where, text) {
	const old = read(where, true);
	const maxLineLength = [...text.split(/\n+/), ...old.split(/\n+/)].reduce((a, b) => Math.max(a, b.length), 0);
	const width = Math.max(52, maxLineLength) + 8;
	const style = "\x1b[2m";
	const reset = "\x1b[0m";
	const padEnds = style + "".padStart(width, "━") + reset;
	const padMid =  style + "".padStart(width, "─") + reset;
	const [paddedText, paddedOld] = [text, old].map(s => "    " + s.replace(/\n/g, "\n    ").replace(/\n *$/, ""));
	let debugOut = `${padEnds}\n`;
	if (preview) debugOut += " \x1b[31m✕\x1b[0m ";
	debugOut += "\x1b[1;3" + (old? "3" : "2") + "m";
	debugOut += where + reset + "\n";
	if (old) debugOut += [padMid, paddedOld, ""].join("\n");
	debugOut += [padMid, paddedText, padEnds].join("\n");
	let spl = debugOut.replace(/\t/g, "    ").split(/\n/);
	const snip = s => s.substr(style.length, s.length - style.length - reset.length);
	if (3 <= spl.length) {
		const last = spl.length - 1;
		spl[0]    = style + "┏" + snip(spl[0])    + "┓" + reset;
		spl[last] = style + "┗" + snip(spl[last]) + "┛" + reset;
		for (let i = 1; i < last; ++i) {
			if (spl[i] == padMid) {
				spl[i] = style + "┠" + snip(spl[i]) + "┨" + reset;
			} else {
				spl[i] = style + "┃" + reset + spl[i] + `\x1b[${width + 2}G` + style + "┃" + reset;
			}
		}
	}
	console.log(["", ...spl, ""].join("\n"));
	if (!preview) {
		fs.mkdirSync(path.dirname(where), {recursive: true});
		fs.writeFileSync(where, text);
	}
}
