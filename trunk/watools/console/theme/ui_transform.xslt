<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="html" indent="yes"/>

<xsl:template match="/">
	<xsl:apply-templates select="/plugin"/>
</xsl:template>

<xsl:template match="plugin">
	<table width="100%" border="1">
		<xsl:apply-templates select="category"/>
	</table>
</xsl:template>

<xsl:template match="category">
	<tr><td colspan="2"><span><xsl:attribute name="onclick">return toggleView('<xsl:value-of select="/plugin/@id" />_<xsl:value-of select="@name" />');</xsl:attribute></span>
			<xsl:value-of select="@label" />
	</td></tr>
	<tr><td colspan="2">
		<table width="100%" border="1">
			<xsl:apply-templates select="*"/>
		</table>
	</td></tr>
</xsl:template>

<xsl:template match="option">
	<xsl:choose><xsl:when test="@composed = 'true'">
		<tr><td colspan="2">+ <b><xsl:value-of select="@label" /></b></td></tr>
		<tr><td colspan="2">
			<table width="100%" border="1">
				<xsl:apply-templates select="*"/>
			</table>
		</td></tr>
	</xsl:when>
	<xsl:otherwise>
		<tr>
		<td><b><xsl:value-of select="@label" /></b></td>
		<td><xsl:choose><xsl:when test="@control = 'bool'">
			<input type="checkbox"></input>
		</xsl:when>
		<xsl:otherwise>
			<input type="text"><xsl:attribute name="value"><xsl:value-of select="text()" /></xsl:attribute></input>
		</xsl:otherwise>
		</xsl:choose>
		</td></tr>
	</xsl:otherwise>
	</xsl:choose>
</xsl:template>
</xsl:stylesheet>
