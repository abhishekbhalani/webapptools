<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="html" indent="yes"/>

<xsl:template match="/">
	<xsl:apply-templates select="/plugin"/>
</xsl:template>

<xsl:template match="plugin">
	<table width="100%">
		<xsl:apply-templates select="*"/>
	</table>
</xsl:template>

<xsl:template match="category">
	<xsl:variable name="ui_code"><xsl:value-of select="/plugin/@id" />_<xsl:value-of select="translate(@name, '/', '_')" /></xsl:variable>
	<tr class='ui-state-hover'><td colspan="2"><span class='ui-icon ui-icon-btn ui-icon-minus'>
		<xsl:attribute name="onclick">return toggleShow('<xsl:value-of select="$ui_code" />');</xsl:attribute>
		<xsl:attribute name="id">btn_<xsl:value-of select="$ui_code" /></xsl:attribute>
		+</span>
			<xsl:value-of select="@label" />
	</td></tr>
	<tr visible="1">
		<xsl:attribute name="id">ui_<xsl:value-of select="$ui_code" /></xsl:attribute>
		<td colspan="2" style="padding-left: 16px;">
		<table width="100%">
			<xsl:apply-templates select="*"/>
		</table>
	</td></tr>
</xsl:template>

<xsl:template match="option">
	<xsl:choose><xsl:when test="@composed = 'true'">
		<xsl:variable name="ui_code"><xsl:value-of select="/plugin/@id" />_<xsl:value-of select="translate(@name, '/', '_')" /></xsl:variable>
		<tr class='ui-state-hover'><td colspan="2"><span class='ui-icon ui-icon-btn ui-icon-minus'>
			<xsl:attribute name="onclick">return toggleShow('<xsl:value-of select="$ui_code" />');</xsl:attribute>
			<xsl:attribute name="id">btn_<xsl:value-of select="$ui_code" /></xsl:attribute>
			+</span><xsl:value-of select="@label" /></td></tr>
		<tr visible="1">
			<xsl:attribute name="id">ui_<xsl:value-of select="$ui_code" /></xsl:attribute>
			<td colspan="2" style="padding-left: 16px;">
			<table width="100%">
				<xsl:apply-templates select="*"/>
			</table>
		</td></tr>
	</xsl:when>
	<xsl:otherwise>
		<tr>
		<xsl:choose>
		<xsl:when test="@control = 'none'">
			<td colspan="2"><b><xsl:value-of select="@label" /></b></td><td></td>
		</xsl:when>
		<xsl:otherwise>
			<td style="white-space: nowrap; text-align: left;width: 50;"><b><xsl:value-of select="@label" /></b></td>
			<td><xsl:variable name="ui_id"><xsl:value-of select="translate(@name, '/', '_')" /></xsl:variable>
			<xsl:choose>
			<xsl:when test="@control = 'select'">
				<select><xsl:attribute name="id"><xsl:value-of select="$ui_id" /></xsl:attribute>
					<xsl:attribute name="name"><xsl:value-of select="@name" /></xsl:attribute>
					<xsl:apply-templates select="select_option"/>
				</select>
			</xsl:when>
			<xsl:when test="@control = 'textarea'">
				<textarea><xsl:attribute name="id"><xsl:value-of select="$ui_id" /></xsl:attribute>
					<xsl:attribute name="name"><xsl:value-of select="@name" /></xsl:attribute>
				<xsl:value-of select="text()" />
				</textarea>
			</xsl:when>
			<xsl:when test="@control = 'radio'">
				<xsl:variable name="ui_id2"><xsl:value-of select="translate(../@name, '/', '_')" /></xsl:variable>
				<input type="radio">
					<xsl:attribute name="data_type"><xsl:value-of select="../@type" /></xsl:attribute>
					<!-- xsl:attribute name="id"><xsl:value-of select="$ui_id2" /></xsl:attribute -->
					<xsl:attribute name="name"><xsl:value-of select="../@name" /></xsl:attribute>
					<xsl:attribute name="value"><xsl:value-of select="@name" /></xsl:attribute>
					<xsl:if test="text() != '0'">
						<xsl:attribute name="checked"></xsl:attribute>
					</xsl:if>
				</input>
			</xsl:when>
			<xsl:when test="@control = 'checkbox'">
				<input type='checkbox'><xsl:attribute name="id"><xsl:value-of select="$ui_id" /></xsl:attribute>
					<xsl:attribute name="name"><xsl:value-of select="@name" /></xsl:attribute>
					<xsl:attribute name="data_type"><xsl:value-of select="@type" /></xsl:attribute>
					<xsl:if test="text() != '0'">
						<xsl:attribute name="checked"></xsl:attribute>
					</xsl:if>
				</input>
			</xsl:when>
			<xsl:otherwise>
				<input><xsl:attribute name="id"><xsl:value-of select="$ui_id" /></xsl:attribute>
					<xsl:attribute name="name"><xsl:value-of select="@name" /></xsl:attribute>
					<xsl:attribute name="type"><xsl:value-of select="@control" /></xsl:attribute>
					<xsl:attribute name="data_type"><xsl:value-of select="@type" /></xsl:attribute>
					<xsl:attribute name="value"><xsl:value-of select="text()" /></xsl:attribute>
				</input>
			</xsl:otherwise>
			</xsl:choose>
			</td>
		</xsl:otherwise>
		</xsl:choose>
		</tr>
	</xsl:otherwise>
	</xsl:choose>
</xsl:template>

<xsl:template match="select_option">
	<option><xsl:attribute name="value"><xsl:value-of select="@value" /></xsl:attribute>
		<xsl:if test="@selected = 'true'">
			<xsl:attribute name="selected"></xsl:attribute>
		</xsl:if>
		<xsl:value-of select="text()" />
	</option>
</xsl:template>
</xsl:stylesheet>
